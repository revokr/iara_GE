#version 330 core
			
layout(location = 0) out vec4 color;

in vec2 v_texCoord;
in vec4 v_color;

uniform vec4 u_color;
uniform float u_tiling_mult;
uniform sampler2D u_texture;

void main() {
	color = texture(u_texture, v_texCoord * u_tiling_mult) * u_color;
}		