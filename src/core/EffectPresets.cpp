#include <core/EffectPresets.h>

#include <fstream>


namespace core
{

const std::vector<EffectPreset>& builtinEffectPresets()
{
    static const std::vector<EffectPreset> presets = {
        { "warm_film", "Warm Film", "Soft contrast, warm grade and light grain", {
            { "film_look", { 1.2f, 0.8f, 0.25f, 0.1f, 0.22f } },
            { "vignette", { 0.35f, 0.55f, 0.45f } },
        } },
        { "cold_grade", "Cold Grade", "Cool temperature with lifted blacks", {
            { "color", { 0.02f, 1.1f, 0.9f, 1.05f, -0.35f, 0.f } },
            { "grain", { 0.15f, 1.2f, 0.3f } },
        } },
        { "vhs_tape", "VHS Tape", "RGB shift, grain and mild CRT", {
            { "rgb_shift", { 4.f, 12.f } },
            { "grain", { 0.45f, 1.5f, 0.2f } },
            { "crt", { 0.35f, 0.15f, 0.1f } },
        } },
        { "crt_monitor", "CRT Monitor", "Scanlines, curvature and glow", {
            { "crt", { 0.55f, 0.35f, 0.25f } },
            { "sharpen", { 0.35f } },
        } },
        { "soft_glow", "Soft Glow", "Light blur and gentle vignette", {
            { "blur", { 6.f } },
            { "vignette", { 0.25f, 0.65f, 0.55f } },
            { "color", { 0.05f, 1.05f, 1.05f, 1.f, 0.05f, 0.f } },
        } },
        { "noir", "Noir", "Desaturated contrast with strong vignette", {
            { "color", { -0.05f, 1.4f, 0.15f, 1.1f, 0.f, 0.f } },
            { "grain", { 0.4f, 1.f, 0.35f } },
            { "vignette", { 0.65f, 0.45f, 0.4f } },
        } },
        { "neon_glitch", "Neon Glitch", "RGB shift and digital glitch blocks", {
            { "glitch", { 0.55f, 20.f, 1.4f } },
            { "rgb_shift", { 6.f, -35.f } },
            { "color", { 0.08f, 1.2f, 1.35f, 1.f, 0.1f, 20.f } },
        } },
        { "clean_punch", "Clean Punch", "Sharper contrast for talking-head cuts", {
            { "color", { 0.03f, 1.25f, 1.05f, 0.95f, 0.05f, 0.f } },
            { "sharpen", { 0.55f } },
        } },
        { "dream_haze", "Dream Haze", "Soft blur, pastel grade and fade", {
            { "blur", { 12.f } },
            { "film_look", { 0.95f, 0.7f, 0.2f, 0.28f, 0.05f } },
        } },
        { "bleach_bypass", "Bleach Bypass", "High contrast, low saturation look", {
            { "color", { 0.f, 1.45f, 0.45f, 1.05f, -0.05f, 0.f } },
            { "grain", { 0.25f, 1.1f, 0.4f } },
        } },
        { "sunset_grade", "Sunset Grade", "Warm orange push with soft vignette", {
            { "color", { 0.06f, 1.15f, 1.1f, 1.f, 0.45f, 8.f } },
            { "vignette", { 0.3f, 0.6f, 0.5f } },
        } },
        { "broadcast_safe", "Broadcast Safe", "Mild contrast and vignette for delivery", {
            { "color", { 0.f, 1.08f, 1.f, 1.f, 0.f, 0.f } },
            { "vignette", { 0.12f, 0.7f, 0.55f } },
        } },
    };

    return presets;
}


const EffectPreset* effectPreset(const std::string& id)
{
    for (const EffectPreset& preset : builtinEffectPresets()) {
        if (preset.id == id) return &preset;
    }

    return nullptr;
}


Json effectPresetToJson(const EffectPreset& preset)
{
    Json json = Json::object();
    json["format"] = "EffeXis effect preset";
    json["version"] = 1;
    json["id"] = preset.id;
    json["name"] = preset.name;
    json["description"] = preset.description;

    Json steps = Json::array();

    for (const EffectPresetStep& step : preset.steps) {
        Json entry = Json::object();
        entry["type"] = step.type;
        Json parameters = Json::array();

        for (float value : step.parameters) {
            parameters.push_back(value);
        }

        entry["parameters"] = parameters;
        steps.push_back(entry);
    }

    json["steps"] = steps;
    return json;
}


bool effectPresetFromJson(const Json& json, EffectPreset& out, std::string& error)
{
    if (json["format"].toString() != "EffeXis effect preset") {
        error = "Not an EffeXis effect preset";
        return false;
    }

    out.id = json["id"].toString();
    out.name = json["name"].toString();
    out.description = json["description"].toString();
    out.steps.clear();

    if (out.id.empty() || out.name.empty()) {
        error = "Preset needs id and name";
        return false;
    }

    for (const Json& entry : json["steps"].items()) {
        EffectPresetStep step;
        step.type = entry["type"].toString();

        if (!videoEffectInfo(step.type)) {
            error = "Unknown effect type: " + step.type;
            return false;
        }

        for (const Json& value : entry["parameters"].items()) {
            step.parameters.push_back((float)value.toDouble());
        }

        out.steps.push_back(step);
    }

    return true;
}


bool loadEffectPresetFile(const std::filesystem::path& path, EffectPreset& out, std::string& error)
{
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        error = "Could not open preset file";
        return false;
    }

    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    try {
        return effectPresetFromJson(Json::parse(text), out, error);
    }
    catch (const std::exception& exception) {
        error = exception.what();
        return false;
    }
}


bool saveEffectPresetFile(const std::filesystem::path& path, const EffectPreset& preset, std::string& error)
{
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    std::ofstream file(path, std::ios::binary);

    if (!file) {
        error = "Could not write preset file";
        return false;
    }

    file << effectPresetToJson(preset).dump(2);
    return (bool)file;
}


std::vector<VideoEffect> effectsFromPreset(const EffectPreset& preset, Id& nextId)
{
    std::vector<VideoEffect> effects;

    for (const EffectPresetStep& step : preset.steps) {
        VideoEffect effect = makeVideoEffect(nextId++, step.type);
        const VideoEffectInfo* info = effect.info();

        if (!info) continue;

        for (size_t i = 0; i < info->parameters.size() && i < step.parameters.size() && i < effect.parameters.size(); i++) {
            effect.parameters[i] = AnimatedValue(step.parameters[i]);
        }

        effects.push_back(effect);
    }

    return effects;
}

}
