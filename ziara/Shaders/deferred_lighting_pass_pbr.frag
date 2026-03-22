#version 450 core
#define NR_POINT_LIGHTS 3
#define MAX_MATERIALS 10
#define MAX_LIGHTS 10

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gDiffuseSpec;
layout(binding = 3) uniform sampler2D gMetallic;
layout(binding = 4) uniform sampler2D gRoughness;
layout(binding = 5) uniform sampler2D shadow_map;
layout(binding = 6) uniform isampler2D entityID_map;
layout(binding = 7) uniform sampler2D ssao_map;

struct PointLight {
	vec4 position;
	
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	float constant;
	float linear;
	float quadratic;

	float padding;
};

struct DirLight {
	vec4 direction;
	
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	bool activeSkyLight;
};

layout (std140, binding = 9) uniform PointLightsUBO {
	PointLight lights[MAX_LIGHTS];
	int nrLights;
};

layout(std140, binding = 6) uniform Camera {
	mat4 u_ViewProjection;
	mat4 u_view;
	vec4 u_camPos;
};

layout(std140, binding = 11) uniform lightSpaceMatrix {
	mat4 u_LightViewProjection;
};

layout(std140, binding = 16) uniform Ambient {
	bool use_ssao;
};

const float PI = 3.14159265;

vec3 fresnelSchilck(float cos_theta, vec3 F0);
float distributionGGX(vec3 normal, vec3 H, float roughness);
float geometrySchilckGGX(float NdotV, float roughness);
float geometrySmith(vec3 normal, vec3 V, vec3 L, float roughness);

void main() {
	
	int entity = texture(entityID_map, TexCoord).r;

	vec3 Pos = texture(gPosition, TexCoord).rgb;
	vec3 world_pos = (inverse(u_view) * vec4(Pos, 1.0)).xyz;
    vec3 Normal = texture(gNormal, TexCoord).rgb;
	vec3 V = normalize(vec3(u_camPos) - world_pos);
    vec3 Diffuse = texture(gDiffuseSpec, TexCoord).rgb;
	float ambient_occlusion = texture(ssao_map, TexCoord).r;
	float metallic = texture(gMetallic, TexCoord).r;
	float roughness = texture(gRoughness, TexCoord).r;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, Diffuse, metallic);

	vec3 Lo = vec3(0.0);
	for (int i = 0; i < nrLights; i++) {
		vec3 L = normalize(lights[i].position.xyz - world_pos);
		vec3 H = normalize(V + L);

		float dist = length(lights[i].position.xyz - world_pos);
		float attenuation = 1.0 / (dist * dist);
		vec3 radiance = lights[i].diffuse.xyz * attenuation;

		float NDF = distributionGGX(Normal, H, roughness);
		float G   = geometrySmith(Normal, V, L, roughness);
		vec3 F    = fresnelSchilck(max(dot(H, V), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		vec3 num = NDF * G * F;
		float denom = 4.0 * max(dot(Normal, V), 0.0) * max(dot(Normal, L), 0.0) + 0.00001;
		vec3 specular = num / denom;

		float NdotL = max(dot(Normal, L), 0.0);
		Lo += (kD * Diffuse / PI + specular) * radiance * NdotL;
	}

	vec3 ambient = vec3(0.03) * Diffuse * ambient_occlusion;
	
	color = vec4(ambient + Lo, 1.0);
}


vec3 fresnelSchilck(float cos_theta, vec3 F0) {
	return F0 * (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

float distributionGGX(vec3 normal, vec3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(normal, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float geometrySchilckGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float geometrySmith(vec3 normal, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(normal, V), 0.0);
	float NdotL = max(dot(normal, L), 0.0);

	float ggx2 = geometrySchilckGGX(NdotV, roughness);
	float ggx1 = geometrySchilckGGX(NdotL, roughness);

	return ggx1 * ggx2;
}