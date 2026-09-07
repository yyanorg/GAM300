#include "pch.h"
#include "Telemetry/TelemetrySystem.hpp"

#include "ECS/ECSRegistry.hpp"
#include "ECS/ECSManager.hpp"
#include "ECS/NameComponent.hpp"
#include "Transform/TransformComponent.hpp"
#include "Script/ScriptComponentData.hpp"
#include "Scene/SceneManager.hpp"
#include "TimeManager.hpp"
#include "Scripting.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

namespace {

    bool g_initialised = false;
    bool g_enabled = false;
    std::string g_path;
    std::ofstream g_out;
    double g_accumulator = 0.0;
    unsigned long long g_frame = 0;
    std::chrono::steady_clock::time_point g_start;

    // Sample rate. Fast enough to close a movement loop, slow enough that
    // the file stays readable over a long run.
    constexpr double kSampleInterval = 0.25;

    // Script basenames we care about. Entities are identified by which
    // script is attached rather than by name or tag, because enemies are
    // prefab instances whose names repeat across the level.
    constexpr const char* kPlayerScript = "PlayerHealth";
    constexpr const char* kEnemyScript = "EnemyAI";
    constexpr const char* kMinibossScript = "MinibossAI";

    std::string BaseName(const std::string& path) {
        const size_t slash = path.find_last_of("/\\");
        std::string file = (slash == std::string::npos) ? path : path.substr(slash + 1);
        const size_t dot = file.find_last_of('.');
        return (dot == std::string::npos) ? file : file.substr(0, dot);
    }

    // ── JSON emission ────────────────────────────────────────────────────
    // Hand-rolled because the engine has no JSON writer on this path and a
    // telemetry sink should not pull one in.

    void AppendEscaped(std::string& out, const std::string& s) {
        out += '"';
        for (const char c : s) {
            switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
            }
        }
        out += '"';
    }

    void AppendNumber(std::string& out, double v, int decimals = 3) {
        if (!std::isfinite(v)) { out += "null"; return; }
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.*f", decimals, v);
        out += buf;
    }

    // ── Lua instance field reads ─────────────────────────────────────────
    // Every helper below leaves the Lua stack exactly as it found it. A
    // read-only observer that unbalances the stack would corrupt the very
    // run it is supposed to be measuring.

    bool PushInstance(lua_State* L, int instanceRef) {
        if (!L || instanceRef == LUA_NOREF || instanceRef == LUA_REFNIL) return false;
        lua_rawgeti(L, LUA_REGISTRYINDEX, instanceRef);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            return false;
        }
        return true;
    }

    bool FieldNumber(lua_State* L, int instanceRef, const char* key, double& out) {
        if (!PushInstance(L, instanceRef)) return false;
        lua_getfield(L, -1, key);
        const bool ok = lua_isnumber(L, -1) != 0;
        if (ok) out = static_cast<double>(lua_tonumber(L, -1));
        lua_pop(L, 2);
        return ok;
    }

    bool FieldBool(lua_State* L, int instanceRef, const char* key, bool& out) {
        if (!PushInstance(L, instanceRef)) return false;
        lua_getfield(L, -1, key);
        const bool present = !lua_isnil(L, -1);
        if (present) out = lua_toboolean(L, -1) != 0;
        lua_pop(L, 2);
        return present;
    }

    bool FieldString(lua_State* L, int instanceRef, const char* key, std::string& out) {
        if (!PushInstance(L, instanceRef)) return false;
        lua_getfield(L, -1, key);
        const bool ok = lua_isstring(L, -1) != 0;
        if (ok) out = lua_tostring(L, -1);
        lua_pop(L, 2);
        return ok;
    }

    // Reads instance[outer][inner] as a string, for fsm.currentName.
    bool FieldNestedString(lua_State* L, int instanceRef, const char* outer, const char* inner, std::string& out) {
        if (!PushInstance(L, instanceRef)) return false;
        lua_getfield(L, -1, outer);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 2);
            return false;
        }
        lua_getfield(L, -1, inner);
        const bool ok = lua_isstring(L, -1) != 0;
        if (ok) out = lua_tostring(L, -1);
        lua_pop(L, 3);
        return ok;
    }

    // Yaw about Y, in degrees, from the world rotation quaternion.
    double YawDegrees(const Quaternion& q) {
        const double siny_cosp = 2.0 * (static_cast<double>(q.w) * q.y + static_cast<double>(q.x) * q.z);
        const double cosy_cosp = 1.0 - 2.0 * (static_cast<double>(q.y) * q.y + static_cast<double>(q.z) * q.z);
        return std::atan2(siny_cosp, cosy_cosp) * 180.0 / 3.14159265358979323846;
    }

    struct Actor {
        Entity entity = 0;
        std::string name;
        Vector3D pos{};
        double yaw = 0.0;
        int instanceRef = LUA_NOREF;
    };

    void AppendActorCommon(std::string& out, const Actor& a) {
        out += "{\"id\":";
        out += std::to_string(a.entity);
        out += ",\"name\":";
        AppendEscaped(out, a.name);
        out += ",\"x\":"; AppendNumber(out, a.pos.x);
        out += ",\"y\":"; AppendNumber(out, a.pos.y);
        out += ",\"z\":"; AppendNumber(out, a.pos.z);
        out += ",\"yaw\":"; AppendNumber(out, a.yaw, 1);
    }

}  // namespace

namespace Telemetry {

    bool IsEnabled() { return g_enabled; }

    const std::string& OutputPath() { return g_path; }

    void Initialise() {
        if (g_initialised) return;
        g_initialised = true;

        const char* flag = std::getenv("GAM300_TELEMETRY");
        if (!flag || std::string(flag) != "1") {
            g_enabled = false;
            return;
        }

        const char* path = std::getenv("GAM300_TELEMETRY_PATH");
        g_path = (path && *path) ? path : "telemetry.jsonl";

        g_out.open(g_path, std::ios::out | std::ios::trunc);
        if (!g_out.is_open()) {
            std::fprintf(stderr, "[telemetry] cannot open '%s' for writing; disabled\n", g_path.c_str());
            g_enabled = false;
            return;
        }

        g_enabled = true;
        g_start = std::chrono::steady_clock::now();
        std::fprintf(stderr, "[telemetry] enabled, writing to %s\n", g_path.c_str());
    }

    void Shutdown() {
        if (g_out.is_open()) {
            g_out.flush();
            g_out.close();
        }
        g_enabled = false;
    }

    void Sample() {
        if (!g_enabled) return;

        ++g_frame;
        g_accumulator += TimeManager::GetUnscaledDeltaTime();
        if (g_accumulator < kSampleInterval) return;
        g_accumulator = 0.0;

        ECSManager& ecs = ECSRegistry::GetInstance().GetActiveECSManager();
        lua_State* L = Scripting::GetLuaState();

        Actor player;
        bool havePlayer = false;
        std::vector<std::pair<Actor, const char*>> enemies;  // actor, script kind

        for (const Entity entity : ecs.GetAllEntities()) {
            auto scriptOpt = ecs.TryGetComponent<ScriptComponentData>(entity);
            if (!scriptOpt.has_value()) continue;

            const ScriptComponentData& sc = scriptOpt.value().get();
            const char* kind = nullptr;
            int instanceRef = LUA_NOREF;

            for (const ScriptData& sd : sc.scripts) {
                if (!sd.enabled) continue;
                const std::string base = BaseName(sd.scriptPath);
                if (base == kPlayerScript)        kind = kPlayerScript;
                else if (base == kEnemyScript)    kind = kEnemyScript;
                else if (base == kMinibossScript) kind = kMinibossScript;
                else continue;
                if (sd.instanceCreated) instanceRef = sd.instanceId;
                break;
            }
            if (!kind) continue;

            auto transformOpt = ecs.TryGetComponent<Transform>(entity);
            if (!transformOpt.has_value()) continue;
            const Transform& tr = transformOpt.value().get();

            Actor a;
            a.entity = entity;
            a.pos = tr.worldPosition;
            a.yaw = YawDegrees(tr.worldRotation);
            a.instanceRef = instanceRef;
            if (auto nameOpt = ecs.TryGetComponent<NameComponent>(entity); nameOpt.has_value()) {
                a.name = nameOpt.value().get().name;
            }

            if (kind == kPlayerScript) {
                player = a;
                havePlayer = true;
            } else {
                enemies.emplace_back(a, kind);
            }
        }

        std::string line;
        line.reserve(2048);

        const double elapsed =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - g_start).count();

        line += "{\"t\":"; AppendNumber(line, elapsed, 2);
        line += ",\"frame\":"; line += std::to_string(g_frame);
        line += ",\"scene\":";
        AppendEscaped(line, SceneManager::GetInstance().GetSceneName());

        line += ",\"player\":";
        if (havePlayer) {
            AppendActorCommon(line, player);
            double hp = 0.0, maxHp = 0.0;
            bool god = false;
            if (FieldNumber(L, player.instanceRef, "CurrentHealth", hp)) {
                line += ",\"hp\":"; AppendNumber(line, hp, 1);
            }
            if (FieldNumber(L, player.instanceRef, "MaxHealth", maxHp)) {
                line += ",\"max_hp\":"; AppendNumber(line, maxHp, 1);
            }
            if (FieldBool(L, player.instanceRef, "GodMode", god)) {
                line += ",\"godmode\":";
                line += god ? "true" : "false";
            }
            line += "}";
        } else {
            line += "null";
        }

        line += ",\"enemies\":[";
        bool first = true;
        int alive = 0;
        for (const auto& [a, kind] : enemies) {
            if (!first) line += ",";
            first = false;
            AppendActorCommon(line, a);

            line += ",\"kind\":";
            AppendEscaped(line, kind == kMinibossScript ? "Miniboss" : "Enemy");

            std::string type;
            if (FieldString(L, a.instanceRef, "EnemyType", type)) {
                line += ",\"type\":";
                AppendEscaped(line, type);
            }
            double hp = 0.0;
            if (FieldNumber(L, a.instanceRef, "health", hp)) {
                line += ",\"hp\":"; AppendNumber(line, hp, 1);
            }
            bool dead = false;
            const bool haveDead = FieldBool(L, a.instanceRef, "dead", dead);
            if (haveDead) {
                line += ",\"dead\":";
                line += dead ? "true" : "false";
            }
            if (!haveDead || !dead) ++alive;

            std::string state;
            if (FieldNestedString(L, a.instanceRef, "fsm", "currentName", state)) {
                line += ",\"state\":";
                AppendEscaped(line, state);
            }
            line += "}";
        }
        line += "],\"enemies_alive\":";
        line += std::to_string(alive);
        line += "}\n";

        g_out << line;
        g_out.flush();  // the reader is another process tailing the file
    }

}  // namespace Telemetry
