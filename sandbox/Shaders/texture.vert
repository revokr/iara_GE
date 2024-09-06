#version 330 core
			
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_tex;


out vec2 v_texCoord;

uniform mat4 u_VP;
uniform mat4 u_transform;	

void main() {
	v_texCoord = a_tex;
	gl_Position = u_VP * u_transform *  vec4(a_pos, 1.0);
}