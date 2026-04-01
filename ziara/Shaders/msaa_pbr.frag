#version 450 core
#define NR_POINT_LIGHTS 3
#define MAX_MATERIALS 10
#define MAX_LIGHTS 10

layout(location = 0) out vec4 color;
layout(location = 1) out int entityID;

layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 Pos;
layout(location = 3) in vec4 camPos;
layout(location = 4) in vec4 pos_light_space;
layout(location = 5) in vec3 T;
layout(location = 6) in vec3 B;
layout(location = 7) in flat int v_EntityID;

struct Material {
	vec4 albedo;
	float shininess;
	vec3 padding;
};

layout(binding = 0) uniform sampler2D diffuse_map;
layout(binding = 1) uniform sampler2D specular_map;
layout(binding = 2) uniform sampler2D normal_map;
layout(binding = 3) uniform sampler2D shadow_map;
layout(binding = 4) uniform sampler2D metallic_map;
layout(binding = 5) uniform sampler2D roughness_map;
layout(binding = 6) uniform sampler2D ssao_map;


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

layout (std140, binding = 8) uniform MaterialUBO {
	Material material;
};

layout (std140, binding = 9) uniform PointLightsUBO {
	PointLight lights[MAX_LIGHTS];
	int nrLights;
};

layout (std140, binding = 10) uniform DirLightUBO {
	DirLight skyLight;
};

layout(std140, binding = 6) uniform Camera {
	mat4 u_ViewProjection;
	mat4 u_view;
	vec4 u_camPos;
};

layout(std140, binding = 7) uniform Model {
	mat4 u_Model;
};

const float PI = 3.14159265;

vec3 getNormalFromMap();
float shadowCalculation();
vec3 fresnelSchilck(float cos_theta, vec3 F0);
float distributionGGX(vec3 normal, vec3 H, float roughness);
float geometrySchilckGGX(float NdotV, float roughness);
float geometrySmith(vec3 normal, vec3 V, vec3 L, float roughness);

void main() {

	vec3 N = normalize(Normal);
	mat3 TBN = mat3(T, B, N);

	vec3 Normal = texture(normal_map, TexCoord).xyz * 2.0 - 1.0;
	Normal = normalize(TBN * Normal);

	vec3 viewDir = normalize(vec3(camPos) - Pos);

	vec3 sun_direction = -skyLight.direction.xyz;
	vec3 light_dir = normalize((u_view * vec4(sun_direction, 0.0)).xyz);

	vec3 viewPos = (u_view * vec4(Pos, 1.0)).xyz;
	vec3 V = normalize(-viewPos);
	vec3 Diffuse = texture(diffuse_map, TexCoord).xyz;
	float ambient_occlusion = texture(ssao_map, TexCoord).x;
	float metallic = texture(metallic_map, TexCoord).x;
	float roughness = texture(roughness_map, TexCoord).x;
	roughness = clamp(roughness, 0.2, 1.0);

	vec3 F0 = vec3(0.04);
	vec3 Lo = vec3(0.0);
	
	F0 = mix(F0, Diffuse, metallic);

	vec3 L = normalize(-light_dir);
	vec3 H = normalize(V + L);

	float attenuation = 1.0;
	vec3 radiance = vec3(1.0) * attenuation;

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
	

	/// Here i try to implement Light_Radius and Light_Size for better specular visuals
	for (int i = 0; i < nrLights; i++) {
		vec3 lightViewPos = (u_view * lights[i].position).xyz;
		vec3 L = normalize(lightViewPos - viewPos);
		vec3 H = normalize(V + L);

		float dist = length(lightViewPos - viewPos);
		float attenuation = 1.0 / (lights[i].constant + lights[i].linear * dist + lights[i].quadratic * dist * dist);
		vec3 radiance = lights[i].diffuse.xyz * attenuation;

		/// Specifically here
		float effective_roughness = clamp(roughness * roughness + 0.19 / dist, 0.0, 1.0);
		effective_roughness = roughness;

		float NDF = distributionGGX(Normal, H, effective_roughness);
		float G   = geometrySmith(Normal, V, L, effective_roughness);
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
	

	vec3 ambient = vec3(0.1) * Diffuse;
	
	color = vec4(ambient + Lo, 1.0);
	entityID = v_EntityID;
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

vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normal_map, TexCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(Pos);
    vec3 Q2  = dFdy(Pos);
    vec2 st1 = dFdx(TexCoord);
    vec2 st2 = dFdy(TexCoord);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN2 = mat3(T, B, N);

    return normalize(TBN2 * tangentNormal);
}

float shadowCalculation() {
    vec3 projCoords = pos_light_space.xyz / pos_light_space.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadow_map, projCoords.xy).x; 
	float currentDepth = projCoords.z;

	float bias = 0.0025;

	float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
    for(int x = -2; x <= 2; ++x)
    {
        for(int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(shadow_map, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 25.0;

	return shadow;
}
