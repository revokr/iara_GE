#version 440

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 TexCoord;


layout(std140, binding = 25) uniform buf {
	vec2 resolution;
	vec2 mouse_pos;
	float t;
};

void main()
{
    gl_Position = vec4(pos, 1.0);
}
