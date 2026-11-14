#version 460

#include "effect_common.glsl"

// Block glitch; a: amount, block size (px), speed.


float hash(float n)
{
    return fract(sin(n) * 43758.5453);
}


void main()
{
    const float block = max(parameters.a.y, 2.0);
    const float row = floor(inUV.y / (parameters.texel.y * block));
    const float slice = hash(row * 17.0 + parameters.a.z * 13.0);
    const float kick = (slice - 0.5) * 2.0 * parameters.a.x * 0.08;
    vec2 uv = inUV + vec2(kick, 0.0);

    if (slice > 0.82) {
        uv.x += (hash(row) - 0.5) * parameters.a.x * 0.12;
    }

    vec4 color = texture(source, uv);

    if (slice > 0.9) {
        color.rgb = color.brg;
    }

    outColor = color;
}
