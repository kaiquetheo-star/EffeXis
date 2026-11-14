#version 460

#include "effect_common.glsl"

// Film grain; a: amount, size, softness.


float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}


void main()
{
    const vec4 color = texture(source, inUV);
    const float size = max(parameters.a.y, 0.5);
    const vec2 cell = floor(inUV / (parameters.texel * size));
    float noise = hash(cell) * 2.0 - 1.0;
    noise = mix(noise, noise * noise * sign(noise), parameters.a.z);
    const vec3 graded = color.rgb + noise * parameters.a.x * 0.35;
    outColor = vec4(graded, color.a);
}
