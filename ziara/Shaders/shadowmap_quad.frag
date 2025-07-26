#version 440 core

layout(location = 0) out vec4 FragColor;
  
layout(location = 0) in vec2 TexCoords;

layout(binding = 0) uniform sampler2D depthMap;

void main()
{             
    float depth = texture(depthMap, TexCoords).x;
    FragColor = vec4(vec3(depth), 1.0);
}  