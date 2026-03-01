#version 450 core
#define NR_POINT_LIGHTS 3
#define MAX_MATERIALS 10
#define MAX_LIGHTS 10

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gDiffuseSpec;
layout(binding = 3) uniform sampler2D shadow_map;
layout(binding = 4) uniform isampler2D entityID_map;
layout(binding = 5) uniform sampler2D ssao_map;

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

layout (std140, binding = 10) uniform DirLightUBO {
	DirLight skyLight;
};

layout(std140, binding = 6) uniform Camera
{
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


vec3 calc_dir_light(DirLight dlight, vec3 viewDir, vec3 normal, vec3 Diffuse, float Specular, vec4 light_pos, vec3 Pos);
vec3 calc_point_light(PointLight light, vec3 normal, vec3 crntPos, vec3 viewDir, vec3 Diffuse, float Specular);
float shadowCalculation(vec4 light_pos);

void main() {
	
	int entity = texture(entityID_map, TexCoord).r;

	vec3 Pos = texture(gPosition, TexCoord).rgb;
    vec3 Normal = texture(gNormal, TexCoord).rgb;
    vec3 Diffuse = texture(gDiffuseSpec, TexCoord).rgb;
    float Specular = texture(gDiffuseSpec, TexCoord).a;
	float ambient_occlusion = texture(ssao_map, TexCoord).r;

	vec4 worldPos = inverse(u_view) * vec4(Pos, 1.0);
	vec4 pos_light_space = u_LightViewProjection * vec4(worldPos);
	vec3 lighting  = Diffuse * 0.3;

	if (use_ssao) lighting *= ambient_occlusion;
    vec3 viewDir = normalize(-Pos);

	/// Directional Light
	vec3 result = lighting;
	if (skyLight.activeSkyLight == true) {
		result += calc_dir_light(skyLight, viewDir, Normal, Diffuse, Specular, pos_light_space, Pos) * 0.2;
	} else {
		result = lighting;
	}
	/// Point lights
	for (int i = 0; i < nrLights; i++) {
		result += calc_point_light(lights[i], Normal, Pos, viewDir, Diffuse, Specular);
	}

	color = vec4(result, 1.0);

	/// Quick solution for correct rendering of SKYBOX
	if (entity > 10000) {
		color = vec4(Diffuse, 1.0);
	}
}

vec3 calc_dir_light(DirLight dlight, vec3 viewDir, vec3 normal, vec3 Diffuse, float Specular, vec4 light_pos, vec3 Pos) {
	vec3 dir = vec3(-dlight.direction);
	vec3 lightDir = normalize(mat3(u_view) * dir);

	float diff = max(dot(lightDir, normal), 0.0); 
	vec3 reflectionDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(viewDir, reflectionDir), 0.0), 10.0);

	vec3 diffuse = vec3(dlight.diffuse) * diff * Diffuse;
	vec3 specular = vec3(dlight.specular) * spec * Specular;

	float shadow = shadowCalculation(light_pos);

	return /*(1.0 - shadow + 0.2) */ (diffuse + specular);
}

vec3 calc_point_light(PointLight light, vec3 normal, vec3 crntPos, vec3 viewDir, vec3 Diffuse, float Specular) {
	vec3 lightDir = normalize(vec3(light.position) - crntPos);

	//vec3 reflectionDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 15.0);


	vec3 diffuse = max(dot(lightDir, normal), 0.0) * Diffuse * vec3(light.diffuse); 
	vec3 specular = vec3(light.specular) * spec * Specular;

	float distance = length(vec3(light.position) - crntPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	if (attenuation < 0.001) attenuation = 0.0;

	diffuse *= attenuation;
	specular *= attenuation;

	return (diffuse + specular);
}

float shadowCalculation(vec4 light_pos) {
    vec3 projCoords = light_pos.xyz / light_pos.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadow_map, projCoords.xy).x; 
	float currentDepth = projCoords.z;

	float bias = 0.0025;

	float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
    for(int x = -2; x <= 2; ++x) {
        for(int y = -2; y <= 2; ++y) {
            float pcfDepth = texture(shadow_map, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 25.0;

	return shadow;
}