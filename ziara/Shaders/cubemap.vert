#version 450 core
layout (location = 0) in vec3 aPos;

layout(std140, binding = 20) uniform CameraCube
{
	mat4 u_ViewProjection;
};

layout (location = 0) out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    gl_Position = u_ViewProjection * vec4(aPos, 1.0);
}