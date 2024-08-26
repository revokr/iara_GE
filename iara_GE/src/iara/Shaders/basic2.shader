#shader vertex
#version 330 core
			
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec4 a_color;		

out vec3 v_pos;
out vec4 v_color;

uniform mat4 u_VP;
uniform mat4 u_transform;	

void main() {
	v_pos = a_pos;
	v_color = a_color;
	gl_Position = u_VP * u_transform *  vec4(a_pos, 1.0);
}


#shader fragment
#version 330 core
			
layout(location = 0) out vec4 color;

in vec3 v_pos;

void main() {
	color = vec4(1.0, 0.4, 0.5, 1.0);
}		