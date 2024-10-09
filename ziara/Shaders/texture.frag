#version 450 core
			
layout(location = 0) out vec4 color;
layout(location = 1) out int red_int;

in vec2  v_texCoord;
in float v_tex_id;
in vec4  v_color;
in float v_tiling;
in flat int v_entityID;

uniform float     u_tiling_mult;
uniform sampler2D u_textures[32];

void main() {
	color = texture(u_textures[int(v_tex_id)], v_texCoord * v_tiling) * v_color;

	red_int = v_entityID; 
}