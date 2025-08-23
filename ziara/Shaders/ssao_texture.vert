#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

layout(location = 0) out vec2 TexCoords;

layout(binding = 0) uniform sampler2D gPosition;

layout (std140, binding = 14) uniform ProjectionUBO {
	mat4 u_view;
	mat4 u_projection;
};

void main() {
    TexCoords = aTexCoords;
    gl_Position = texture(gPosition, aTexCoords);
}