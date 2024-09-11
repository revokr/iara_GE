#version 330 core
			
layout(location = 0) out vec4 color;

in vec2  v_texCoord;
in float v_tex_id;
in vec4  v_color;
in float v_tiling;

uniform float     u_tiling_mult;
uniform sampler2D u_textures[32];

void main() {
	color = texture(u_textures[int(v_tex_id)], v_texCoord * v_tiling) * v_color;
}