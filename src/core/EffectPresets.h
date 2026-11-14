#pragma once

#include <core/Json.h>
#include <core/VideoEffects.h>

#include <filesystem>
#include <string>
#include <vector>


namespace core
{
    struct EffectPresetStep
    {
        std::string type;
        std::vector<float> parameters;
    };


    struct EffectPreset
    {
        std::string id;
        std::string name;
        std::string description;
        std::vector<EffectPresetStep> steps;
    };


    // Built-in EffeXis looks (clean-room), plus presets loaded from a folder.
    const std::vector<EffectPreset>& builtinEffectPresets();
    const EffectPreset* effectPreset(const std::string& id);

    // Load/save a single .effexis-preset.json file.
    bool loadEffectPresetFile(const std::filesystem::path& path, EffectPreset& out, std::string& error);
    bool saveEffectPresetFile(const std::filesystem::path& path, const EffectPreset& preset, std::string& error);

    // Apply a preset as concrete VideoEffect instances (caller assigns ids).
    std::vector<VideoEffect> effectsFromPreset(const EffectPreset& preset, Id& nextId);

    Json effectPresetToJson(const EffectPreset& preset);
    bool effectPresetFromJson(const Json& json, EffectPreset& out, std::string& error);
}
