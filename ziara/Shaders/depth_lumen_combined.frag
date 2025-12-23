#version 450 core

layout(location = 0) out float color;

layout(location = 0) in vec2 TexCoord;

layout(binding = 0) uniform sampler2D previus_frame;
layout(binding = 1) uniform sampler2D second_previus_frame;
layout(binding = 2) uniform sampler2D current_frame;

void main() {
	vec2 texel_size = 1.0 / vec2(textureSize(previus_frame, 0));
	float result1 = 0.0;
	float result2 = 0.0;
	float result3 = 0.0;
	float max1 = 0.0;
	float max2 = 0.0;
	for (int x = -2; x < 2; x++) {
		for (int y = -2; y < 2; y++) {
			vec2 offset = vec2(float(x), float(y)) * texel_size;
			if (texture(previus_frame, TexCoord).r != 0.0)
				result1 += texture(previus_frame, TexCoord + offset).r;
			if (texture(second_previus_frame, TexCoord).r != 0.0)
				result2 += texture(second_previus_frame, TexCoord + offset).r;
			max1 = max(max1,texture(previus_frame, TexCoord + offset).r);
			max2 = max(max2,texture(second_previus_frame, TexCoord + offset).r);
		}
	}
	result1 /= 16;
	result2 /= 16;
	color = (result1 + result2 + texture(current_frame, TexCoord).r) / 3.0;
	//color = (texture(current_frame, TexCoord).r + max(max1, max2)) / 2.0;
	

	//color = (texture(second_previus_frame, TexCoord).r + texture(previus_frame, TexCoord).r + texture(current_frame, TexCoord).r) /3.0;
}