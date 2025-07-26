#version 450 core
layout(location = 0) in vec3 a_pos;

layout(std140, binding = 11) uniform lightSpaceMatrix
{
	mat4 u_LightViewProjection;
};

layout(std140, binding = 12) uniform Model {
	mat4 u_Model;
};

void main() {
    gl_Position = u_LightViewProjection * u_Model * vec4(a_pos, 1.0);
} 