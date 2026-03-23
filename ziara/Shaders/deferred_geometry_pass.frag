#version 450 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 T;
layout(location = 4) in vec3 B;
layout(location = 5) in flat int v_EntityID;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gDiffuseSpec;
layout(location = 3) out int entityID;

layout(binding = 0) uniform sampler2D diffuse_map;
layout(binding = 1) uniform sampler2D specular_map;
layout(binding = 2) uniform sampler2D normal_map;
layout(binding = 3) uniform sampler2D metallic_map;
layout(binding = 4) uniform sampler2D roughness_map;

void main() {
	gPosition.rgb = Pos;
	gPosition.a   = texture(metallic_map, TexCoord).r;

	vec3 N = normalize(Normal);
	mat3 TBN = mat3(T, B, N);
	vec3 normal = texture(normal_map, TexCoord).xyz * 2.0 - 1.0;
	gNormal.rgb = vec3(normalize(TBN * normal));
	gNormal.a   = texture(roughness_map, TexCoord).r;


	gDiffuseSpec.rgb = texture(diffuse_map, TexCoord).rgb;
	gDiffuseSpec.a   = texture(specular_map, TexCoord).r;
 
	entityID = v_EntityID;
}
