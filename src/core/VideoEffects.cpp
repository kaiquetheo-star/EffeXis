#include <core/VideoEffects.h>


namespace core
{

// Icons of the Material Design font (see fonts/IconsMaterialDesign.h).
#define ICON_COLOR "\xee\x90\x8a"       // palette
#define ICON_BLUR "\xee\x8e\xa5"        // blur_on
#define ICON_SHARPEN "\xee\x8f\x88"     // details
#define ICON_VIGNETTE "\xee\x90\xb5"    // vignette
#define ICON_KEY "\xee\x90\xa1"         // texture
#define ICON_PIXELATE "\xee\x8f\xac"    // grid_on
#define ICON_GRAIN "\xee\x8f\x9b"       // grain
#define ICON_SHIFT "\xee\x97\x8e"       // filter_b_and_w
#define ICON_CRT "\xee\x8e\xb0"         // desktop_windows
#define ICON_GLITCH "\xee\x95\x93"      // flash_on
#define ICON_FILM "\xee\x8e\xa0"        // movie


const std::vector<VideoEffectInfo>& videoEffectInfos()
{
    static const std::vector<VideoEffectInfo> infos = {
        { VideoEffectType::ColorCorrection, "color", "Color Correction", ICON_COLOR, {
            { "Brightness", -1.f, 1.f, 0.f, "%+.0f %%", 100.f },
            { "Contrast", 0.f, 3.f, 1.f, "%.0f %%", 100.f },
            { "Saturation", 0.f, 3.f, 1.f, "%.0f %%", 100.f },
            { "Gamma", 0.2f, 3.f, 1.f, "%.2f" },
            { "Temperature", -1.f, 1.f, 0.f, "%+.0f", 100.f },
            { "Hue", -180.f, 180.f, 0.f, "%+.0f\xc2\xb0" },
        } },
        { VideoEffectType::Blur, "blur", "Blur", ICON_BLUR, {
            { "Radius", 0.f, 200.f, 10.f, "%.1f px" },
        } },
        { VideoEffectType::Sharpen, "sharpen", "Sharpen", ICON_SHARPEN, {
            { "Amount", 0.f, 3.f, 0.5f, "%.0f %%", 100.f },
        } },
        { VideoEffectType::Vignette, "vignette", "Vignette", ICON_VIGNETTE, {
            { "Amount", 0.f, 1.f, 0.5f, "%.0f %%", 100.f },
            { "Size", 0.f, 1.f, 0.6f, "%.0f %%", 100.f },
            { "Softness", 0.01f, 1.f, 0.5f, "%.0f %%", 100.f },
        } },
        { VideoEffectType::ChromaKey, "chromakey", "Chroma Key", ICON_KEY, {
            { "Red", 0.f, 1.f, 0.f, "%.2f" },
            { "Green", 0.f, 1.f, 1.f, "%.2f" },
            { "Blue", 0.f, 1.f, 0.f, "%.2f" },
            { "Threshold", 0.f, 1.f, 0.25f, "%.0f %%", 100.f },
            { "Softness", 0.f, 1.f, 0.1f, "%.0f %%", 100.f },
            { "Spill", 0.f, 1.f, 0.5f, "%.0f %%", 100.f },
        }, true },
        { VideoEffectType::Pixelate, "pixelate", "Pixelate", ICON_PIXELATE, {
            { "Size", 1.f, 200.f, 16.f, "%.0f px" },
        } },
        { VideoEffectType::Grain, "grain", "Film Grain", ICON_GRAIN, {
            { "Amount", 0.f, 1.f, 0.35f, "%.0f %%", 100.f },
            { "Size", 0.5f, 4.f, 1.f, "%.2f" },
            { "Softness", 0.f, 1.f, 0.25f, "%.0f %%", 100.f },
        } },
        { VideoEffectType::RgbShift, "rgb_shift", "RGB Shift", ICON_SHIFT, {
            { "Amount", 0.f, 20.f, 3.f, "%.1f px" },
            { "Angle", -180.f, 180.f, 0.f, "%+.0f\xc2\xb0" },
        } },
        { VideoEffectType::Crt, "crt", "CRT", ICON_CRT, {
            { "Scanlines", 0.f, 1.f, 0.45f, "%.0f %%", 100.f },
            { "Curvature", 0.f, 1.f, 0.2f, "%.0f %%", 100.f },
            { "Glow", 0.f, 1.f, 0.15f, "%.0f %%", 100.f },
        } },
        { VideoEffectType::Glitch, "glitch", "Glitch", ICON_GLITCH, {
            { "Amount", 0.f, 1.f, 0.4f, "%.0f %%", 100.f },
            { "BlockSize", 2.f, 64.f, 16.f, "%.0f px" },
            { "Speed", 0.f, 4.f, 1.f, "%.2f" },
        } },
        { VideoEffectType::FilmLook, "film_look", "Film Look", ICON_FILM, {
            { "Contrast", 0.5f, 2.f, 1.15f, "%.0f %%", 100.f },
            { "Saturation", 0.f, 2.f, 0.85f, "%.0f %%", 100.f },
            { "Warmth", -1.f, 1.f, 0.15f, "%+.0f", 100.f },
            { "Fade", 0.f, 1.f, 0.12f, "%.0f %%", 100.f },
            { "Grain", 0.f, 1.f, 0.2f, "%.0f %%", 100.f },
        } },
    };

    return infos;
}


const VideoEffectInfo* videoEffectInfo(const std::string& id)
{
    for (const VideoEffectInfo& info : videoEffectInfos()) {
        if (id == info.id) return &info;
    }

    return nullptr;
}


VideoEffect makeVideoEffect(Id id, const std::string& type)
{
    VideoEffect effect;
    effect.id = id;
    effect.type = type;

    if (const VideoEffectInfo* info = videoEffectInfo(type)) {
        for (const VideoEffectParameter& parameter : info->parameters) {
            effect.parameters.emplace_back(parameter.defaultValue);
        }
    }

    return effect;
}

}
