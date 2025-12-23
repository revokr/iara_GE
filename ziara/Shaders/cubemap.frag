#version 450 core

layout(location = 0) in vec3 TexCoords;
layout(location = 0) out vec4 FragColor;

layout (binding = 0) uniform samplerCube skybox;

layout (std140, binding = 17) uniform SunDirection {
    vec4 u_sunDirection;
};

void main()
{    
    vec3 zenith_color = vec3(0.5, 0.5, 0.9);
    vec3 horizon_color = vec3(0.8, 0.6, 0.7);
    vec3 sun_color = vec3(1.0, 0.9, 0.6);
    
    vec3 dir = normalize(vec3(u_sunDirection));
    float t = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);

    vec3 sky = mix(horizon_color, zenith_color, pow(t, 1.5));

    float sun_intensity = max(dot(dir, normalize(dir)), 0.0);
    float sun_disk = smoothstep(0.9995, 1.0, sun_intensity);
    sky += sun_color;

    FragColor = vec4(sky, 1.0);

    sky = mix(horizon_color, zenith_color, TexCoords.y);
    FragColor = vec4(sky, 1.0);
    FragColor = texture(skybox, TexCoords);
}