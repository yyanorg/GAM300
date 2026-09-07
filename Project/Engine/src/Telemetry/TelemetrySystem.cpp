#include "pch.h"
#include "Telemetry/TelemetrySystem.hpp"

#include "ECS/ECSRegistry.hpp"
#include "ECS/ECSManager.hpp"
#include "ECS/NameComponent.hpp"
#include "Transform/TransformComponent.hpp"
#include "Animation/AnimationComponent.hpp"
#include "Script/ScriptComponentData.hpp"
#include "Scene/SceneManager.hpp"
#include "TimeManager.hpp"
#include "Scripting.h"
#include "Script/LuaBindableSystems.hpp"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <chrono>
#include <cctype>
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
    std::string g_lastScene;
    double g_walkmapPeriod = 0.0;   // seconds; 0 disables
    double g_walkmapAccum = 0.0;
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
    constexpr const char* kInputScript = "InputInterpreter";
    constexpr const char* kChainScript = "ChainBootstrap";

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

    // Reads instance[outer][inner] as a number, for controller.chainLen.
    bool FieldNestedNumber(lua_State* L, int instanceRef, const char* outer, const char* inner, double& out) {
        if (!PushInstance(L, instanceRef)) return false;
        lua_getfield(L, -1, outer);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 2);
            return false;
        }
        lua_getfield(L, -1, inner);
        const bool ok = lua_isnumber(L, -1) != 0;
        if (ok) out = static_cast<double>(lua_tonumber(L, -1));
        lua_pop(L, 3);
        return ok;
    }

    // Reads instance[outer][inner] as a boolean, for controller.isExtending.
    bool FieldNestedBool(lua_State* L, int instanceRef, const char* outer, const char* inner, bool& out) {
        if (!PushInstance(L, instanceRef)) return false;
        lua_getfield(L, -1, outer);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 2);
            return false;
        }
        lua_getfield(L, -1, inner);
        const bool present = !lua_isnil(L, -1);
        if (present) out = lua_toboolean(L, -1) != 0;
        lua_pop(L, 3);
        return present;
    }

    // Guards the crosshair cast: a zero or non-finite forward is meaningless
    // and Jolt will not thank us for it.
    bool phys_ok(double x, double y, double z) {
        const double n = x * x + y * y + z * z;
        return std::isfinite(n) && n > 1e-6;
    }

    bool GlobalNumber(lua_State* L, const char* name, double& out) {
        if (!L) return false;
        lua_getglobal(L, name);
        const bool ok = lua_isnumber(L, -1) != 0;
        if (ok) out = static_cast<double>(lua_tonumber(L, -1));
        lua_pop(L, 1);
        return ok;
    }

    bool GlobalBool(lua_State* L, const char* name, bool& out) {
        if (!L) return false;
        lua_getglobal(L, name);
        const bool present = !lua_isnil(L, -1);
        if (present) out = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);
        return present;
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

    // Mirrors EnemyAI.lua's IsFlying(): strip quotes and whitespace, fold
    // case, compare. One enemy in 04_Level has its EnemyType authored as the
    // literal string "\"Flying\"", quote characters included. The game copes
    // because IsFlying normalises, but a consumer comparing the raw string
    // classifies that enemy as a ground unit. A static scan made exactly that
    // mistake and concluded the statue room had one flying enemy when it has
    // two, so normalise here rather than leave the trap in place downstream.
    bool IsFlyingType(const std::string& raw) {
        std::string t;
        t.reserve(raw.size());
        for (const char c : raw) {
            if (c == '"' || std::isspace(static_cast<unsigned char>(c))) continue;
            t += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return t == "flying";
    }

    // camera_follow.lua publishes its orbit yaw as the global CAMERA_YAW.
    // WASD is camera-relative, so without this a world-space destination
    // cannot be turned into a key to hold: the same key walks a different
    // direction depending on where the camera happens to be pointing.
    bool CameraYaw(lua_State* L, double& out) {
        if (!L) return false;
        lua_getglobal(L, "CAMERA_YAW");
        const bool ok = lua_isnumber(L, -1) != 0;
        if (ok) out = static_cast<double>(lua_tonumber(L, -1));
        lua_pop(L, 1);
        return ok;
    }

    // Yaw about Y, in degrees, from the world rotation quaternion.
    double YawDegrees(const Quaternion& q) {
        const double siny_cosp = 2.0 * (static_cast<double>(q.w) * q.y + static_cast<double>(q.x) * q.z);
        const double cosy_cosp = 1.0 - 2.0 * (static_cast<double>(q.y) * q.y + static_cast<double>(q.z) * q.z);
        return std::atan2(siny_cosp, cosy_cosp) * 180.0 / 3.14159265358979323846;
    }

    // A census is written once per scene load: every named entity with a
    // transform, in WORLD coordinates.
    //
    // This exists because the scene file cannot answer the question. Transform
    // is serialised as localPosition/localScale/localRotation only (see
    // REFL_REGISTER_START(Transform)), so every position on disk is relative
    // to a parent, and rooms nest their contents several levels deep. Reading
    // a door or pickup's world position statically means recomposing the whole
    // ancestor chain. The running game has already done that work, so ask it.
    void WriteCensus(ECSManager& ecs, const std::string& scene) {
        std::string line;
        line.reserve(1 << 16);
        line += "{\"type\":\"census\",\"scene\":";
        AppendEscaped(line, scene);
        line += ",\"entities\":[";

        bool first = true;
        for (const Entity entity : ecs.GetAllEntities()) {
            auto nameOpt = ecs.TryGetComponent<NameComponent>(entity);
            if (!nameOpt.has_value()) continue;
            const std::string& name = nameOpt.value().get().name;
            if (name.empty()) continue;

            auto transformOpt = ecs.TryGetComponent<Transform>(entity);
            if (!transformOpt.has_value()) continue;
            const Transform& tr = transformOpt.value().get();

            if (!first) line += ",";
            first = false;
            line += "{\"id\":";
            line += std::to_string(entity);
            line += ",\"name\":";
            AppendEscaped(line, name);
            line += ",\"x\":"; AppendNumber(line, tr.worldPosition.x);
            line += ",\"y\":"; AppendNumber(line, tr.worldPosition.y);
            line += ",\"z\":"; AppendNumber(line, tr.worldPosition.z);
            line += "}";
        }
        line += "]}\n";
        g_out << line;
        g_out.flush();
    }

    // A local height map, sampled by casting rays straight down on a grid
    // around the player.
    //
    // This exists because neither the scene file nor the engine's own nav
    // data can say where the player may walk. NavGrid is a single XZ layer
    // bounded to +/-20 with one ground height per cell, so it cannot
    // represent a balcony above a floor and does not reach the statue room
    // at x 23 to 42 at all. Without geometry, an external driver navigates by
    // walking into walls and backing off, which is slow and cannot tell a
    // staircase from a bookshelf.
    //
    // A downward ray reports the top of whatever occupies a cell, so a wall
    // reads as a tall column and a step reads as a small rise. That is enough
    // to route: a neighbouring cell is reachable when its height differs from
    // the current one by less than the character's step height.
    void WriteWalkmap(const Vector3D& centre) {
        PhysicsSystem* phys = PhysicsSystemWrappers::g_PhysicsSystem;
        if (!phys) return;

        constexpr int kHalf = 24;          // cells each side of the player
        constexpr float kCell = 0.5f;      // world units per cell
        // Start just above head height, not higher. Storeys in this level are
        // about 3.27 apart, so a ray starting several units up begins above
        // the ceiling and reports the floor of the storey above instead of
        // the one the player is standing on.
        constexpr float kUp = 1.5f;
        constexpr float kDown = 8.0f;

        std::string line;
        line.reserve(1 << 15);
        line += "{\"type\":\"walkmap\",\"cx\":"; AppendNumber(line, centre.x);
        line += ",\"cz\":"; AppendNumber(line, centre.z);
        line += ",\"cy\":"; AppendNumber(line, centre.y);
        line += ",\"cell\":"; AppendNumber(line, kCell, 2);
        line += ",\"half\":"; line += std::to_string(kHalf);
        line += ",\"h\":[";

        const Vector3D down(0.0f, -1.0f, 0.0f);
        bool first = true;
        for (int gz = -kHalf; gz <= kHalf; ++gz) {
            for (int gx = -kHalf; gx <= kHalf; ++gx) {
                const float wx = centre.x + gx * kCell;
                const float wz = centre.z + gz * kCell;
                const Vector3D origin(wx, centre.y + kUp, wz);
                const auto r = phys->Raycast(origin, down, kDown);
                if (!first) line += ",";
                first = false;
                if (r.hit) {
                    AppendNumber(line, centre.y + kUp - r.distance, 2);
                } else {
                    line += "null";
                }
            }
        }
        line += "]}\n";
        g_out << line;
        g_out.flush();
    }

    struct Actor {
        Entity entity = 0;
        std::string name;
        Vector3D pos{};
        double yaw = 0.0;
        int instanceRef = LUA_NOREF;
        // The animation state machine's current state, and which clip is
        // actually playing. Reported alongside the AI's own FSM state so the
        // two can be compared: "the enemy is idle but playing the attack
        // clip" is a claim about the relationship between them, and without
        // both it can only be judged by eye from a screenshot.
        bool haveAnim = false;
        std::string animState;
        long long animClip = -1;
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

        // Optional, and off by default: sampling a 49x49 grid is 2401 raycasts,
        // which is cheap at a fraction of a hertz and wasteful every frame.
        if (const char* wm = std::getenv("GAM300_TELEMETRY_WALKMAP")) {
            g_walkmapPeriod = std::atof(wm);
            if (g_walkmapPeriod < 0.0) g_walkmapPeriod = 0.0;
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

        const std::string scene = SceneManager::GetInstance().GetSceneName();
        if (scene != g_lastScene) {
            g_lastScene = scene;
            WriteCensus(ecs, scene);
        }

        Actor player;
        bool havePlayer = false;
        int inputRef = LUA_NOREF;
        bool haveInput = false;
        int chainRef = LUA_NOREF;
        bool haveChain = false;
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
                else if (base == kInputScript)    kind = kInputScript;
                else if (base == kChainScript)    kind = kChainScript;
                else continue;
                if (sd.instanceCreated) instanceRef = sd.instanceId;
                break;
            }
            if (!kind) continue;

            // InputInterpreter is not an actor; it is picked up here only
            // because this is the one pass over the entity list.
            if (kind == kInputScript) {
                inputRef = instanceRef;
                haveInput = true;
                continue;
            }
            if (kind == kChainScript) {
                chainRef = instanceRef;
                haveChain = true;
                continue;
            }

            auto transformOpt = ecs.TryGetComponent<Transform>(entity);
            if (!transformOpt.has_value()) continue;
            const Transform& tr = transformOpt.value().get();

            Actor a;
            a.entity = entity;
            a.pos = tr.worldPosition;
            a.yaw = YawDegrees(tr.worldRotation);
            a.instanceRef = instanceRef;
            if (auto animOpt = ecs.TryGetComponent<AnimationComponent>(entity); animOpt.has_value()) {
                AnimationComponent& anim = animOpt.value().get();
                a.haveAnim = true;
                a.animState = anim.GetCurrentState();
                a.animClip = static_cast<long long>(anim.GetActiveClipIndex());
            }
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
        AppendEscaped(line, scene);

        double camYaw = 0.0;
        if (CameraYaw(L, camYaw)) {
            line += ",\"camera_yaw\":";
            AppendNumber(line, camYaw, 2);
        }
        double camPitch = 0.0;
        if (GlobalNumber(L, "CAMERA_PITCH", camPitch)) {
            line += ",\"camera_pitch\":";
            AppendNumber(line, camPitch, 2);
        }
        // The camera's world position and unit forward vector, which
        // camera_follow.lua already publishes. The chain fires along this
        // forward, so having it turns aiming from guess-and-check into a
        // measurable angular error against the direction to the target.
        double fx = 0.0, fy = 0.0, fz = 0.0;
        if (GlobalNumber(L, "CAMERA_FWD_X", fx)
            && GlobalNumber(L, "CAMERA_FWD_Y", fy)
            && GlobalNumber(L, "CAMERA_FWD_Z", fz)) {
            line += ",\"camera_fwd\":[";
            AppendNumber(line, fx, 4); line += ",";
            AppendNumber(line, fy, 4); line += ",";
            AppendNumber(line, fz, 4);
            line += "]";
        }
        double px = 0.0, py = 0.0, pz = 0.0;
        const bool havePos = GlobalNumber(L, "CAMERA_POS_X", px)
                             && GlobalNumber(L, "CAMERA_POS_Y", py)
                             && GlobalNumber(L, "CAMERA_POS_Z", pz);
        if (havePos) {
            line += ",\"camera_pos\":[";
            AppendNumber(line, px); line += ",";
            AppendNumber(line, py); line += ",";
            AppendNumber(line, pz);
            line += "]";
        }

        // What the crosshair is actually on.
        //
        // This is the quantity that decides whether a hook connects, and it
        // is not the same as where the camera is pointed. camera_chain_aim.lua
        // raycasts from the camera along its forward, and publishes the hit
        // point as the chain's world target; ChainBootstrap then fires from
        // the player's HAND toward that point. So a shot lands on an enemy
        // only when the camera ray strikes the enemy's collider. A ray that
        // passes a few centimetres over it takes its world target from the
        // wall eight units behind, and the chain flies to that wall instead,
        // which is exactly what a dozen failed hooks on the second statue
        // room flyer were doing.
        if (havePos && phys_ok(fx, fy, fz)) {
            PhysicsSystem* phys = PhysicsSystemWrappers::g_PhysicsSystem;
            if (phys) {
                const auto hit = phys->Raycast(Vector3D(static_cast<float>(px),
                                                        static_cast<float>(py),
                                                        static_cast<float>(pz)),
                                               Vector3D(static_cast<float>(fx),
                                                        static_cast<float>(fy),
                                                        static_cast<float>(fz)),
                                               100.0f);
                line += ",\"crosshair\":";
                if (hit.hit) {
                    line += "{\"dist\":"; AppendNumber(line, hit.distance);
                    line += ",\"x\":"; AppendNumber(line, hit.hitPoint.x);
                    line += ",\"y\":"; AppendNumber(line, hit.hitPoint.y);
                    line += ",\"z\":"; AppendNumber(line, hit.hitPoint.z);
                    line += ",\"entity\":"; line += std::to_string(hit.entityId);
                    line += "}";
                } else {
                    line += "null";
                }
            }
        }

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
            // Feather count and the skill's cost, both Lua globals. The
            // feather skill's only externally visible effect is this number
            // going down, so without it there is no way to tell a cast that
            // fired and did nothing from a keypress the game ignored.
            double feathers = 0.0;
            if (GlobalNumber(L, "_numFeathers", feathers)) {
                line += ",\"feathers\":"; AppendNumber(line, feathers, 0);
            }
            double featherCost = 0.0;
            if (GlobalNumber(L, "_featherSkillRequirement", featherCost)) {
                line += ",\"feather_skill_cost\":"; AppendNumber(line, featherCost, 0);
            }
            line += "}";
        } else {
            line += "null";
        }

        // Whether the game is currently ignoring input matters as much as
        // where the player is. A cinematic freeze looks identical to walking
        // into a wall from the outside: position stops changing while keys
        // are held. Telling them apart from telemetry stops a driver from
        // reporting "stuck" when the right response is "wait".
        if (haveInput) {
            bool frozen = false, dead = false;
            const bool haveFrozen = FieldBool(L, inputRef, "_frozenByCinematic", frozen);
            const bool haveDead = FieldBool(L, inputRef, "_playerDead", dead);
            if (haveFrozen || haveDead) {
                line += ",\"input\":{\"frozen\":";
                line += (haveFrozen && frozen) ? "true" : "false";
                line += ",\"dead\":";
                line += (haveDead && dead) ? "true" : "false";
                line += "}";
            }
        }

        // Chain state. Without it the hook is opaque: a shot that never fired,
        // one that fired into a wall, and one that hit an enemy all look
        // identical from outside, because the only visible consequence of a
        // successful hook is a change on the enemy that a miss does not
        // produce either.
        if (haveChain) {
            bool aiming = false;
            const bool haveAim = GlobalBool(L, "CHAIN_AIM_ACTIVE", aiming);
            double len = 0.0;
            const bool haveLen = FieldNestedNumber(L, chainRef, "controller", "chainLen", len);
            bool extending = false, locked = false, snapped = false;
            const bool haveExt = FieldNestedBool(L, chainRef, "controller", "isExtending", extending);
            FieldNestedBool(L, chainRef, "controller", "endPointLocked", locked);
            FieldNestedBool(L, chainRef, "controller", "_raycastSnapped", snapped);
            if (haveAim || haveLen || haveExt) {
                line += ",\"chain\":{\"aiming\":";
                line += aiming ? "true" : "false";
                line += ",\"len\":"; AppendNumber(line, len, 2);
                line += ",\"extending\":";
                line += extending ? "true" : "false";
                line += ",\"locked\":";
                line += locked ? "true" : "false";
                line += ",\"wall\":";
                line += snapped ? "true" : "false";
                line += "}";
            }
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
                line += ",\"flying\":";
                line += IsFlyingType(type) ? "true" : "false";
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
            if (a.haveAnim) {
                line += ",\"anim\":";
                AppendEscaped(line, a.animState);
                line += ",\"anim_clip\":";
                line += std::to_string(a.animClip);
            }
            line += "}";
        }
        line += "],\"enemies_alive\":";
        line += std::to_string(alive);
        line += "}\n";

        g_out << line;
        g_out.flush();  // the reader is another process tailing the file

        if (g_walkmapPeriod > 0.0 && havePlayer) {
            g_walkmapAccum += kSampleInterval;
            if (g_walkmapAccum >= g_walkmapPeriod) {
                g_walkmapAccum = 0.0;
                WriteWalkmap(player.pos);
            }
        }
    }

}  // namespace Telemetry
