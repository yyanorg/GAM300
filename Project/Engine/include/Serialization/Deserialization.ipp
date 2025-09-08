#pragma once

#include "Deserialization.hpp"

//#include "../System/TransformationSystem.hpp"
//#include "../ECS/Component.hpp"

namespace deserializer {

#if 0
    template <typename T>
    T deserializeComponent(rapidjson::Value const& jsonObj) {
        using namespace rapidjson;
        T component{};

        if (jsonObj.IsObject()) {
            for (auto it = jsonObj.MemberBegin(); it != jsonObj.MemberEnd(); ++it) {
                const std::string key = it->name.GetString();
                auto const& node = it->value;

                reflection::recursivelyIterate(component, [&](auto fieldData) {
                    auto& dataMember = fieldData.get();
                    using DataMemberType = std::remove_cv_t<std::remove_reference_t<decltype(dataMember)>>;

                    if (fieldData.name() != key) return;

                    if constexpr (std::same_as<DataMemberType, Vector2D>) {
                        dataMember = { node["x"].GetFloat(), node["y"].GetFloat() };
                    }
                    else if constexpr (std::same_as<DataMemberType, Vector3D>) {
                        dataMember = { node["r"].GetFloat(), node["g"].GetFloat(), node["b"].GetFloat() };
                    }
                    else if constexpr (std::same_as<DataMemberType, Vector4D>) {
                        dataMember = { node["r"].GetFloat(), node["g"].GetFloat(), node["b"].GetFloat(), node["a"].GetFloat() };
                    }
                    else if constexpr (std::same_as<DataMemberType, ShapeData::Shape>) {
                        const std::string s = node.GetString();
                        if (s == "Point")       dataMember = ShapeData::Shape::POINT;
                        else if (s == "Line")   dataMember = ShapeData::Shape::LINE;
                        else if (s == "Circle") dataMember = ShapeData::Shape::CIRCLE;
                        else                    dataMember = ShapeData::Shape::RECTANGLE;
                    }
                    else if constexpr (std::same_as<DataMemberType, Rigidbody::Material>) {
                        const std::string s = node.GetString();
                        if (s == "Invisible")       dataMember = Rigidbody::Material::INVISIBLE;
                        else if (s == "Dynamic")     dataMember = Rigidbody::Material::DYNAMIC;
                        else                         dataMember = Rigidbody::Material::STATIC;
                    }
                    else if constexpr (std::same_as<DataMemberType, Transform::Coordinate>) {
                        dataMember = (std::string(node.GetString()) == "Viewport")
                            ? Transform::Coordinate::Viewport
                            : Transform::Coordinate::World;
                    }
                    else if constexpr (std::same_as<DataMemberType, Light::AttenuationFormula>) {
                        dataMember = (std::string(node.GetString()) == "Constant")
                            ? Light::AttenuationFormula::Constant
                            : Light::AttenuationFormula::Quadratic;
                    }
                    else if constexpr (std::same_as<DataMemberType, ParticleEmitter::Type>) {
                        const std::string s = node.GetString();
                        if (s == "Image")               dataMember = ParticleEmitter::Type::Image;
                        else if (s == "CircleGradient") dataMember = ParticleEmitter::Type::CircleGradient;
                        else if (s == "Square")         dataMember = ParticleEmitter::Type::Square;
                        else if (s == "Animation")      dataMember = ParticleEmitter::Type::Animation;
                        else                            dataMember = ParticleEmitter::Type::Circle;
                    }
                    else if constexpr (std::same_as<DataMemberType, unsigned>) {
                        dataMember = node.GetUint();
                    }
                    else if constexpr (std::same_as<DataMemberType, int>) {
                        dataMember = node.GetInt();
                    }
                    else if constexpr (std::same_as<DataMemberType, float>) {
                        dataMember = node.GetFloat();
                    }
                    else if constexpr (std::same_as<DataMemberType, double>) {
                        dataMember = node.GetDouble();
                    }
                    else if constexpr (std::same_as<DataMemberType, std::string>) {
                        dataMember = node.GetString();
                    }
                    else if constexpr (std::same_as<DataMemberType, bool>) {
                        dataMember = node.GetBool();
                    }
                    else if constexpr (std::same_as<DataMemberType, std::size_t>) {
                        dataMember = static_cast<std::size_t>(node.GetUint64());
                    }

                    // ----- Composite: vector<ScriptData::IndividualScript>
                    else if constexpr (std::same_as<DataMemberType, std::vector<ScriptData::IndividualScript>>) {
                        std::vector<ScriptData::IndividualScript> vec;
                        for (auto const& jsonElement : node.GetArray()) {
                            std::vector<ScriptData::IndividualScript::ExternalVariable> variables;
                            for (auto const& jsonVariable : jsonElement["variables"].GetArray()) {
                                std::variant<std::string, double, bool> value;
                                const std::string type = jsonVariable["type"].GetString();
                                if (type == "string") {
                                    value = std::string(jsonVariable["value"].GetString());
                                }
                                else if (type == "double") {
                                    value = jsonVariable["value"].GetDouble();
                                }
                                else if (type == "bool") {
                                    value = jsonVariable["value"].GetBool();
                                }
                                variables.push_back({ jsonVariable["name"].GetString(), std::move(value) });
                            }
                            vec.push_back({
                                jsonElement["id"].GetUint(),
                                std::move(variables),
                                jsonElement["pause"].GetBool()
                                });
                        }
                        dataMember = std::move(vec);
                    }

                    // ----- Composite: vector<AudioData::IndividualAudio>
                    else if constexpr (std::same_as<DataMemberType, std::vector<AudioData::IndividualAudio>>) {
                        std::vector<AudioData::IndividualAudio> audios;
                        for (auto const& jsonElement : node.GetArray()) {
                            AudioData::IndividualAudio::Group audioGroup;
                            const std::string g = jsonElement["group"].GetString();
                            if (g == "music")    audioGroup = AudioData::IndividualAudio::Group::MUSIC;
                            else if (g == "sfx")      audioGroup = AudioData::IndividualAudio::Group::SFX;
                            else if (g == "reserve1") audioGroup = AudioData::IndividualAudio::Group::RESERVE1;
                            else if (g == "reserve2") audioGroup = AudioData::IndividualAudio::Group::RESERVE2;
                            else                      audioGroup = AudioData::IndividualAudio::Group::RESERVE3;

                            audios.push_back({
                                jsonElement["id"].GetUint(),
                                jsonElement["name"].GetString(),
                                jsonElement["loop"].GetBool(),
                                jsonElement["toPlayAtStart"].GetBool(),
                                jsonElement["locality"].GetBool(),
                                jsonElement["persistence"].GetBool(),
                                jsonElement["scenePersistence"].GetBool(),
                                jsonElement["volume"].GetFloat(),
                                jsonElement["pitch"].GetFloat(),
                                jsonElement["distanceMultiplier"].GetFloat(),
                                audioGroup
                                });
                        }
                        dataMember = std::move(audios);
                    }

                    // ----- CameraData::VignettePostProcessing
                    else if constexpr (std::same_as<DataMemberType, CameraData::VignettePostProcessing>) {
                        auto const& j = node["vignette"];
                        dataMember = {
                            j["enabled"].GetBool(),
                            j["offset"].GetFloat(),
                            { j["color"]["x"].GetFloat(), j["color"]["y"].GetFloat(), j["color"]["z"].GetFloat() }
                        };
                    }

                    // ----- CameraData::LightPostProcessing
                    else if constexpr (std::same_as<DataMemberType, CameraData::LightPostProcessing>) {
                        auto const& j = node["light"];
                        dataMember = {
                            j["enabled"].GetBool(),
                            { j["color"]["x"].GetFloat(), j["color"]["y"].GetFloat(), j["color"]["z"].GetFloat() },
                            j["ambient"].GetFloat()
                        };
                    }

                    // ----- CameraData::FogPostProcessing
                    else if constexpr (std::same_as<DataMemberType, CameraData::FogPostProcessing>) {
                        auto const& j = node["fog"];
                        dataMember = {
                            j["enabled"].GetBool(),
                            j["persistence"].GetFloat(),
                            j["octaves"].GetInt(),
                            j["speed"].GetFloat(),
                            { j["color"]["x"].GetFloat(), j["color"]["y"].GetFloat(), j["color"]["z"].GetFloat(), j["color"]["w"].GetFloat() }
                        };
                    }

                    // ----- CameraData::ChromaticAbberation
                    else if constexpr (std::same_as<DataMemberType, CameraData::ChromaticAbberation>) {
                        auto const& j = node["chromatic"];
                        dataMember = {
                            j["enabled"].GetBool(),
                            j["offset"].GetFloat(),
                            j["variance"].GetFloat(),
                            j["time"].GetFloat()
                        };
                    }

                    // ----- CameraData::InversePostProcessing
                    else if constexpr (std::same_as<DataMemberType, CameraData::InversePostProcessing>) {
                        auto const& j = node["inverse"];
                        dataMember = { j["enabled"].GetBool() };
                    }

                    // ----- CameraData::GrayScalePostProcessing
                    else if constexpr (std::same_as<DataMemberType, CameraData::GrayScalePostProcessing>) {
                        auto const& j = node["grayScale"];
                        dataMember = { j["enabled"].GetBool() };
                    }

                    // ----- CameraData::SharpenPostProcessing
                    else if constexpr (std::same_as<DataMemberType, CameraData::SharpenPostProcessing>) {
                        auto const& j = node["sharpen"];
                        dataMember = { j["enabled"].GetBool(), j["intensity"].GetFloat() };
                    }

                    // ----- CameraData::BlurPostProcessing
                    else if constexpr (std::same_as<DataMemberType, CameraData::BlurPostProcessing>) {
                        auto const& j = node["blur"];
                        dataMember = { j["enabled"].GetBool() };
                    }

                    // ----- CameraData::SobelEdgeDetection
                    else if constexpr (std::same_as<DataMemberType, CameraData::SobelEdgeDetection>) {
                        auto const& j = node["sobel"];
                        dataMember = { j["enabled"].GetBool() };
                    }

                    }); // recursivelyIterate
            } // for each member
        } // if object

        if constexpr (std::same_as<T, Transform>) {
            component.model = TransformationSystem::calculateModelMatrix(
                component.position, component.scale, component.rotation, component.height);
        }

        return component;
    }

} // namespace deserializer


namespace serializer {

    template <typename T>
    std::tuple<std::string, rapidjson::Value, std::vector<AssetID>>
        serializeComponent(T const& component, rapidjson::Document::AllocatorType& alloc) {
        using namespace rapidjson;

        Value componentJson(kObjectType);
        std::vector<AssetID> assets;

        if constexpr (reflection::isReflectable<T>) {
            reflection::recursivelyIterate(component, [&](auto const& fieldData) {
                auto const& dataMember = fieldData.get();
                using DataMemberType = std::remove_cv_t<std::remove_reference_t<decltype(dataMember)>>;
                const char* key = fieldData.name();

                auto addString = [&](const std::string& s) {
                    Value k(key, alloc);
                    Value v;
                    v.SetString(s.c_str(), static_cast<SizeType>(s.size()), alloc);
                    componentJson.AddMember(k, v, alloc);
                    };

                auto addObj = [&]() {
                    Value k(key, alloc);
                    Value v(kObjectType);
                    componentJson.AddMember(k, v, alloc);
                    return componentJson.FindMember(key)->value;
                    };

                // Collect asset ids for lazy loading (unchanged behavior)
                if constexpr ((std::same_as<T, AnimationData> || std::same_as<T, ImageData>) &&
                    std::same_as<DataMemberType, AssetID>) {
                    assets.push_back(dataMember);
                }

                if constexpr (std::same_as<DataMemberType, Vector2D>) {
                    Value v(kObjectType);
                    v.AddMember("x", dataMember.x(), alloc);
                    v.AddMember("y", dataMember.y(), alloc);
                    componentJson.AddMember(Value(key, alloc), v, alloc);
                }
                else if constexpr (std::same_as<DataMemberType, Vector3D>) {
                    Value v(kObjectType);
                    v.AddMember("r", dataMember.x(), alloc);
                    v.AddMember("g", dataMember.y(), alloc);
                    v.AddMember("b", dataMember.z(), alloc);
                    componentJson.AddMember(Value(key, alloc), v, alloc);
                }
                else if constexpr (std::same_as<DataMemberType, Vector4D>) {
                    Value v(kObjectType);
                    v.AddMember("r", dataMember.x(), alloc);
                    v.AddMember("g", dataMember.y(), alloc);
                    v.AddMember("b", dataMember.z(), alloc);
                    v.AddMember("a", dataMember.w(), alloc);
                    componentJson.AddMember(Value(key, alloc), v, alloc);
                }
                else if constexpr (std::same_as<DataMemberType, ShapeData::Shape>) {
                    switch (dataMember) {
                        using enum ShapeData::Shape;
                    case POINT:     addString("Point"); break;
                    case LINE:      addString("Line"); break;
                    case CIRCLE:    addString("Circle"); break;
                    default:
                    case RECTANGLE: addString("Rectangle"); break;
                    }
                }
                else if constexpr (std::same_as<DataMemberType, Rigidbody::Material>) {
                    switch (dataMember) {
                        using enum Rigidbody::Material;
                    case STATIC:    addString("Static"); break;
                    case DYNAMIC:   addString("Dynamic"); break;
                    case INVISIBLE: addString("Invisible"); break;
                    }
                }
                else if constexpr (std::same_as<DataMemberType, Transform::Coordinate>) {
                    switch (dataMember) {
                        using enum Transform::Coordinate;
                    case World:    addString("World"); break;
                    case Viewport: addString("Viewport"); break;
                    }
                }
                else if constexpr (std::same_as<DataMemberType, Light::AttenuationFormula>) {
                    addString(dataMember == Light::AttenuationFormula::Constant ? "Constant" : "Quadratic");
                }
                else if constexpr (std::same_as<DataMemberType, ParticleEmitter::Type>) {
                    using enum ParticleEmitter::Type;
                    switch (dataMember) {
                    case Circle:         addString("Circle"); break;
                    case CircleGradient: addString("CircleGradient"); break;
                    case Square:         addString("Square"); break;
                    case Image:          addString("Image"); break;
                    case Animation:      addString("Animation"); break;
                    }
                }
                else if constexpr (std::same_as<DataMemberType, std::string>) {
                    addString(dataMember);
                }
                else if constexpr (std::is_arithmetic_v<DataMemberType> || std::same_as<DataMemberType, bool>) {
                    componentJson.AddMember(Value(key, alloc), dataMember, alloc);
                }

                // ----- Composite: vector<ScriptData::IndividualScript>
                else if constexpr (std::same_as<DataMemberType, std::vector<ScriptData::IndividualScript>>) {
                    Value arr(kArrayType);
                    for (auto const& script : dataMember) {
                        Value s(kObjectType);
                        s.AddMember("id", script.id, alloc);
                        s.AddMember("pause", script.pause, alloc);

                        Value vars(kArrayType);
                        for (auto const& var : script.variables) {
                            Value vj(kObjectType);
                            // name
                            {
                                Value nameV; nameV.SetString(var.identifier.c_str(), (SizeType)var.identifier.size(), alloc);
                                vj.AddMember("name", nameV, alloc);
                            }
                            // type + value from variant
                            std::visit([&](auto&& arg) {
                                using VT = std::decay_t<decltype(arg)>;
                                if constexpr (std::is_same_v<VT, std::string>) {
                                    Value t; t.SetString("string", alloc);
                                    Value vv; vv.SetString(arg.c_str(), (SizeType)arg.size(), alloc);
                                    vj.AddMember("type", t, alloc);
                                    vj.AddMember("value", vv, alloc);
                                }
                                else if constexpr (std::is_same_v<VT, double>) {
                                    Value t; t.SetString("double", alloc);
                                    vj.AddMember("type", t, alloc);
                                    vj.AddMember("value", arg, alloc);
                                }
                                else if constexpr (std::is_same_v<VT, bool>) {
                                    Value t; t.SetString("bool", alloc);
                                    vj.AddMember("type", t, alloc);
                                    vj.AddMember("value", arg, alloc);
                                }
                                }, var.value);
                            vars.PushBack(vj, alloc);
                        }

                        s.AddMember("variables", vars, alloc);
                        arr.PushBack(s, alloc);
                    }
                    componentJson.AddMember(Value(key, alloc), arr, alloc);
                }

                // ----- Composite: vector<AudioData::IndividualAudio>
                else if constexpr (std::same_as<DataMemberType, std::vector<AudioData::IndividualAudio>>) {
                    Value arr(kArrayType);
                    for (auto const& audio : dataMember) {
                        Value aj(kObjectType);
                        aj.AddMember("id", audio.assetId, alloc);

                        Value nameV; nameV.SetString(audio.name.c_str(), (SizeType)audio.name.size(), alloc);
                        aj.AddMember("name", nameV, alloc);

                        aj.AddMember("loop", audio.loop, alloc);
                        aj.AddMember("toPlayAtStart", audio.toPlayAtStart, alloc);
                        aj.AddMember("locality", audio.soundLocality, alloc);
                        aj.AddMember("persistence", audio.persistUponEntityDeletion, alloc);
                        aj.AddMember("scenePersistence", audio.persistDuringChangeScene, alloc);
                        aj.AddMember("volume", audio.volume, alloc);
                        aj.AddMember("pitch", audio.pitch, alloc);
                        aj.AddMember("distanceMultiplier", audio.distanceMultiplier, alloc);

                        const char* grp =
                            (audio.group == AudioData::IndividualAudio::Group::MUSIC) ? "music" :
                            (audio.group == AudioData::IndividualAudio::Group::SFX) ? "sfx" :
                            (audio.group == AudioData::IndividualAudio::Group::RESERVE1) ? "reserve1" :
                            (audio.group == AudioData::IndividualAudio::Group::RESERVE2) ? "reserve2" : "reserve3";
                        Value g; g.SetString(grp, alloc);
                        aj.AddMember("group", g, alloc);

                        arr.PushBack(aj, alloc);
                    }
                    componentJson.AddMember(Value(key, alloc), arr, alloc);
                }

                // ----- CameraData::VignettePostProcessing
                else if constexpr (std::same_as<DataMemberType, CameraData::VignettePostProcessing>) {
                    Value outer(kObjectType), j(kObjectType), color(kObjectType);
                    j.AddMember("enabled", dataMember.enabled, alloc);
                    color.AddMember("x", dataMember.color.x(), alloc);
                    color.AddMember("y", dataMember.color.y(), alloc);
                    color.AddMember("z", dataMember.color.z(), alloc);
                    j.AddMember("color", color, alloc);
                    j.AddMember("offset", dataMember.radiusOffset, alloc);
                    outer.AddMember("vignette", j, alloc);
                    componentJson.AddMember(Value(key, alloc), outer, alloc);
                }

                // ----- CameraData::LightPostProcessing
                else if constexpr (std::same_as<DataMemberType, CameraData::LightPostProcessing>) {
                    Value outer(kObjectType), j(kObjectType), color(kObjectType);
                    j.AddMember("enabled", dataMember.enabled, alloc);
                    color.AddMember("x", dataMember.color.x(), alloc);
                    color.AddMember("y", dataMember.color.y(), alloc);
                    color.AddMember("z", dataMember.color.z(), alloc);
                    j.AddMember("color", color, alloc);
                    j.AddMember("ambient", dataMember.ambientLightFactor, alloc);
                    outer.AddMember("light", j, alloc);
                    componentJson.AddMember(Value(key, alloc), outer, alloc);
                }

                // ----- CameraData::FogPostProcessing
                else if constexpr (std::same_as<DataMemberType, CameraData::FogPostProcessing>) {
                    Value outer(kObjectType), j(kObjectType), color(kObjectType);
                    j.AddMember("enabled", dataMember.enabled, alloc);
                    j.AddMember("persistence", dataMember.persistence, alloc);
                    j.AddMember("octaves", dataMember.octaves, alloc);
                    j.AddMember("speed", dataMember.speed, alloc);
                    color.AddMember("x", dataMember.color.x(), alloc);
                    color.AddMember("y", dataMember.color.y(), alloc);
                    color.AddMember("z", dataMember.color.z(), alloc);
                    color.AddMember("w", dataMember.color.w(), alloc);
                    j.AddMember("color", color, alloc);
                    outer.AddMember("fog", j, alloc);
                    componentJson.AddMember(Value(key, alloc), outer, alloc);
                }

                // ----- CameraData::ChromaticAbberation
                else if constexpr (std::same_as<DataMemberType, CameraData::ChromaticAbberation>) {
                    Value outer(kObjectType), j(kObjectType);
                    j.AddMember("enabled", dataMember.enabled, alloc);
                    j.AddMember("offset", dataMember.textureOffset, alloc);
                    j.AddMember("variance", dataMember.colorVariance, alloc);
                    j.AddMember("time", dataMember.timeBetweenNewOffset, alloc);
                    outer.AddMember("chromatic", j, alloc);
                    componentJson.AddMember(Value(key, alloc), outer, alloc);
                }

                // ----- CameraData::InversePostProcessing
                else if constexpr (std::same_as<DataMemberType, CameraData::InversePostProcessing>) {
                    Value outer(kObjectType), j(kObjectType);
                    j.AddMember("enabled", dataMember.enabled, alloc);
                    outer.AddMember("inverse", j, alloc);
                    componentJson.AddMember(Value(key, alloc), outer, alloc);
                }

                // ----- CameraData::GrayScalePostProcessing
                else if constexpr (std::same_as<DataMemberType, CameraData::GrayScalePostProcessing>) {
                    Value outer(kObjectType), j(kObjectType);
                    j.AddMember("enabled", dataMember.enabled, alloc);
                    outer.AddMember("grayScale", j, alloc);
                    componentJson.AddMember(Value(key, alloc), outer, alloc);
                }

                // ----- CameraData::SharpenPostProcessing
                else if constexpr (std::same_as<DataMemberType, CameraData::SharpenPostProcessing>) {
                    Value outer(kObjectType), j(kObjectType);
                    j.AddMember("enabled", dataMember.enabled, alloc);
                    j.AddMember("intensity", dataMember.intensity, alloc);
                    outer.AddMember("sharpen", j, alloc);
                    componentJson.AddMember(Value(key, alloc), outer, alloc);
                }

                // ----- CameraData::BlurPostProcessing
                else if constexpr (std::same_as<DataMemberType, CameraData::BlurPostProcessing>) {
                    Value outer(kObjectType), j(kObjectType);
                    j.AddMember("enabled", dataMember.enabled, alloc);
                    outer.AddMember("blur", j, alloc);
                    componentJson.AddMember(Value(key, alloc), outer, alloc);
                }

                // ----- CameraData::SobelEdgeDetection
                else if constexpr (std::same_as<DataMemberType, CameraData::SobelEdgeDetection>) {
                    Value outer(kObjectType), j(kObjectType);
                    j.AddMember("enabled", dataMember.enabled, alloc);
                    outer.AddMember("sobel", j, alloc);
                    componentJson.AddMember(Value(key, alloc), outer, alloc);
                }

                }); // recursivelyIterate
        } // if reflectable

        return { reflection::getName<T>(), componentJson, assets };
    }

#endif

} // namespace serializer