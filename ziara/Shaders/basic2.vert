#version 330 core
			
layout(location = 0) in vec3 a_pos;

out vec3 v_pos;

uniform mat4 u_VP;
uniform mat4 u_transform;	

void main() {
	v_pos = a_pos;
	gl_Position = u_VP * u_transform *  vec4(a_pos, 1.0);
}