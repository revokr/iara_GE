#version 330 core
			
layout(location = 0) out vec4 color;

uniform vec4 u_color;

in vec3 v_pos;
in vec4 v_color;

void main() {
	color = u_color;
}