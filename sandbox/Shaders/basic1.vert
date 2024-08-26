#version 330 core
			
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec4 a_color;		



out vec3 v_pos;
out vec4 v_color;

uniform mat4 u_VP;
uniform mat4 u_transform;	

void main() {
	v_pos = a_pos;
	v_color = u_color;
	gl_Position = u_VP * u_transform *  vec4(a_pos, 1.0);
}