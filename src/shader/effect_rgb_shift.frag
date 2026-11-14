#version 460

#include "effect_common.glsl"

// Chromatic aberration; a: amount (px), angle (degrees).


void main()
{
    const float amount = parameters.a.x * parameters.pixelScale;
    const float angle = radians(parameters.a.y);
    const vec2 offset = vec2(cos(angle), sin(angle)) * amount * parameters.texel;
    const float r = texture(source, inUV + offset).r;
    const float g = texture(source, inUV).g;
    const float b = texture(source, inUV - offset).b;
    const float a = texture(source, inUV).a;
    outColor = vec4(r, g, b, a);
}
