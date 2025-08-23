#version 440 core

layout(location = 0) out vec4 FragColor;
  
layout(location = 0) in vec2 TexCoords;

layout(binding = 0) uniform sampler2D hdr_texture;

layout(std140, binding = 21) uniform Exposure {
    float exposure;
};

vec3 RRTAndODTFit(vec3 v) {
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (v * 0.983729 + 0.4329510) + 0.238081;
    return a / b;
}

vec3 acesFilm(const vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d ) + e), 0.0, 1.0);
}

vec3 Tonemap_Unreal(vec3 x) {
    // Unreal 3, Documentation: "Color Grading"
    // Adapted to be close to Tonemap_ACES, with similar range
    // Gamma 2.2 correction is baked in, don't use with sRGB conversion!
    return x / (x + 0.155) * 1.019;
}

float Tonemap_Lottes(float x) {
    // Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
    const float a = 1.6;
    const float d = 0.977;
    const float hdrMax = 8.0;
    const float midIn = 0.18;
    const float midOut = 0.267;

    // Can be precomputed
    const float b =
        (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
    const float c =
        (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

    return pow(x, a) / (pow(x, a * d) * b + c);
}

void main()
{             
    const float gamma = 2.2;
    vec3 hdrcolor = texture(hdr_texture, TexCoords).rgb;

    // exposure tone mapping
    //vec3 mapped = vec3(1.0) - exp(-hdrcolor * exposure);
    // gamma correction 
    //mapped = pow(mapped, vec3(1.0 / gamma));

    // ACES tone mapping
    vec3 mapped = RRTAndODTFit(hdrcolor * exposure);

    FragColor = vec4(mapped, 1.0);
}  