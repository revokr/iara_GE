#version 450 core
#define NR_POINT_LIGHTS 3
#define MAX_MATERIALS 10
#define MAX_LIGHTS 10

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gDiffuseSpec;
layout(binding = 3) uniform isampler2D entityID_map;
layout(binding = 4) uniform sampler2D ssao_map;

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

layout(std140, binding = 16) uniform Ambient {
	bool use_ssao;
};

layout (std140, binding = 10) uniform DirLightUBO {
	DirLight skyLight;
};

const float PI = 3.14159265;

vec3 fresnelSchilck(float cos_theta, vec3 F0);
float distributionGGX(vec3 normal, vec3 H, float roughness);
float geometrySchilckGGX(float NdotV, float roughness);
float geometrySmith(vec3 normal, vec3 V, vec3 L, float roughness);

void main() {
	
	int entity = texture(entityID_map, TexCoord).r;

	vec3 sun_direction = -skyLight.direction.xyz;

	vec3 light_dir = (u_view * vec4(sun_direction, 0.0)).xyz;

	vec3 viewPos = texture(gPosition, TexCoord).rgb;
    vec3 Normal = normalize(texture(gNormal, TexCoord).rgb);
	vec3 V = normalize(-viewPos);
    vec3 Diffuse = texture(gDiffuseSpec, TexCoord).rgb;
	float ambient_occlusion = texture(ssao_map, TexCoord).r;
	float metallic = texture(gPosition, TexCoord).a;
	float roughness = texture(gNormal, TexCoord).a;
	roughness = clamp(roughness, 0.25, 1.0);

	vec3 F0 = vec3(0.04);
	vec3 Lo = vec3(0.0);
	
	F0 = mix(F0, Diffuse + 0.1, metallic);

	
	vec3 L = normalize(-light_dir);
	vec3 H = normalize(V + L);

	float attenuation = 1.0;
	vec3 radiance = skyLight.diffuse.xyz * attenuation;

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
	

	
	for (int i = 0; i < nrLights; i++) {
		vec3 lightViewPos = (u_view * lights[i].position).xyz;
		vec3 L = normalize(lightViewPos - viewPos);
		vec3 H = normalize(V + L);

		float dist = length(lightViewPos - viewPos);
		float attenuation = 1.0 / (lights[i].constant + lights[i].linear * dist + lights[i].quadratic * dist * dist);
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
	

	vec3 ambient = vec3(0.1) * Diffuse * ambient_occlusion;
	
	color = vec4(ambient + Lo, 1.0);
}


vec3 fresnelSchilck(float cos_theta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
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