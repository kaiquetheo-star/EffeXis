#version 460

#include "effect_common.glsl"

// Soft film grade; a: contrast, saturation, warmth, fade; b: grain.


float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}


void main()
{
    vec4 color = texture(source, inUV);
    vec3 rgb = color.rgb;

    rgb = (rgb - 0.5) * parameters.a.x + 0.5;
    const float luma = dot(rgb, vec3(0.2126, 0.7152, 0.0722));
    rgb = mix(vec3(luma), rgb, parameters.a.y);
    rgb.r += parameters.a.z * 0.08;
    rgb.b -= parameters.a.z * 0.05;
    rgb = mix(rgb, vec3(0.5), parameters.a.w);

    const float noise = hash(floor(inUV / parameters.texel)) * 2.0 - 1.0;
    rgb += noise * parameters.b.x * 0.2;

    outColor = vec4(clamp(rgb, 0.0, 1.0), color.a);
}
