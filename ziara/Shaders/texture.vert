#version 330 core
			
layout(location = 0) in vec3  a_pos;
layout(location = 1) in vec4  a_color;
layout(location = 2) in vec2  a_tex;
layout(location = 3) in float a_tex_id;
layout(location = 4) in float a_tiling_mult;



out vec2  v_texCoord;
out vec4  v_color;
out float v_tex_id;
out float v_tiling;

uniform mat4 u_VP;

void main() {
	v_tex_id = a_tex_id;
	v_texCoord = a_tex;
	v_color = a_color;
	v_tiling = a_tiling_mult;
	gl_Position = u_VP * vec4(a_pos, 1.0);
}