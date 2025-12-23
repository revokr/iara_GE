#version 450 core

layout(location = 0) out float color;

layout(location = 0) in vec2 TexCoord;

layout(binding = 0) uniform sampler2D ssao_input;

void main() {
	vec2 texel_size = 1.0 / vec2(textureSize(ssao_input, 0));
	float result = 0.0;
	for (int x = -2; x < 2; x++) {
		for (int y = -2; y < 2; y++) {
			vec2 offset = vec2(float(x), float(y)) * texel_size;
			result += texture(ssao_input, TexCoord + offset).r;
		}
	}
	color = result / 16.0;
}