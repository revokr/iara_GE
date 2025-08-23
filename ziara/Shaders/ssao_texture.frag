#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 TexCoord;

layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D noiseTexture;
layout(binding = 3) uniform isampler2D entityID_map;

layout (std140, binding = 15) uniform ViewportSizesUBO {
	vec2 u_vp_size;
};

layout (std140, binding = 13) uniform SSAOSamples {
	vec4 u_samples[16];  /// USED AS A VEC3
};

void main() {
	const vec2 noise_scale = vec2(u_vp_size.x / 4.0, u_vp_size.y / 4.0); 

	int entity = texture(entityID_map, TexCoord).r;

	vec3 normal = texture(gNormal, TexCoord).rgb;
	vec3 random_vec = normalize(texture(noiseTexture, TexCoord * noise_scale).rgb);

//	vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
//	vec3 bitangent = cross(normal, tangent);
//	mat3 TBN = mat3(tangent, bitangent, normal);
//
//	float bias = 0.025;
//	float radius = 0.5;
//	float occlusion = 0.0;
//	for (uint i = 0; i < 16; i++) {
//		vec3 sample_pos = TBN * vec3(u_samples[i]);
//		sample_pos = frag_pos + sample_pos * radius;
//
//		vec4 offset = vec4(sample_pos, 1.0);
//		offset      = u_projection * offset;  // from view-space to clip-space
//		offset.xyz /= offset.w;               // perspective divide ???
//		offset.xyz  = offset.xyz * 0.5 + 0.5; // to range 0.0 - 1.0
//
//		float sample_depth = texture(gPosition, offset.xy).z;
//		float range_check  = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
//		occlusion += (sample_depth >= sample_pos.z + bias ? 1.0 : 0.0) * range_check;
//	}

	//color = 1.0 - (occlusion / 16);
	color = vec4(1.0);

	if (entity > 10000) {
		color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}