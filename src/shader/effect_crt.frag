#version 460

#include "effect_common.glsl"

// CRT look; a: scanlines, curvature, glow.


void main()
{
    vec2 uv = inUV;
    const vec2 center = 0.5 * (parameters.visible.xy + parameters.visible.zw);
    vec2 delta = uv - center;
    delta *= 1.0 + parameters.a.y * dot(delta, delta) * 4.0;
    uv = center + delta;

    vec4 color = texture(source, uv);
    const float line = sin((uv.y / max(parameters.texel.y, 0.0001)) * 3.14159265);
    const float scan = mix(1.0, 0.65 + 0.35 * line * line, parameters.a.x);
    color.rgb *= scan;
    color.rgb += color.rgb * parameters.a.z * 0.25;
    outColor = color;
}
