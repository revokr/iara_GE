#version 450 core
#define NR_POINT_LIGHTS 3
#define MAX_MATERIALS 10
#define MAX_LIGHTS 10

layout(location = 0) out vec4 color;
layout(location = 1) out int entityID;

layout (location = 0) in vec2 TexCoord;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 Pos;
layout (location = 3) in vec4 camPos;
layout (location = 4) in flat int v_EntityID;

struct Material {
	vec4 albedo;
	float shininess;
	vec3 padding;
};

layout(binding = 0) uniform sampler2D diffuse_map;
layout(binding = 1) uniform sampler2D specular_map;

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
};

struct SpotLight {
	vec4 position;
	vec4 direction;
	float cut_off;
	float outer_cut_off;
	vec2 padding;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	float constant;
	float linear;
	float quadratic;
	float padding2;
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

vec3 calc_point_light(Material material,PointLight light, vec3 normal, vec3 crntPos, vec3 viewDir);
vec3 calc_dir_light(Material material,DirLight light);

void main() {

	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(vec3(camPos) - Pos);
	vec3 result = calc_dir_light(material, skyLight);
	for (int i = 0; i < nrLights; i++) {
			result += calc_point_light(material, lights[i], norm, Pos, viewDir);
	}
	color = vec4(result, 1.0);
	entityID = v_EntityID;
}

vec3 calc_dir_light(Material material, DirLight dlight) {
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(vec3(camPos) - Pos);
	vec3 dir = vec3(dlight.direction);
	vec3 lightDir = normalize(-dir);

	float diff = max(dot(lightDir, normal), 0.0); 
	vec3 reflectionDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);

 	vec3 ambient = vec3(dlight.ambient) * vec3(material.albedo);
	vec3 diffuse = vec3(dlight.diffuse) * (diff * vec3(texture(diffuse_map, TexCoord)));
	vec3 specular = vec3(dlight.specular) * (spec * vec3(texture(specular_map, TexCoord)));

	return (ambient + diffuse + specular);
}

vec3 calc_point_light(Material material,PointLight light, vec3 normal, vec3 crntPos, vec3 viewDir) {
	vec3 lightDir = normalize(vec3(light.position) - crntPos);

	float diff = max(dot(lightDir, normal), 0.0); 
	vec3 reflectionDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);

 	vec3 ambient = vec3(light.ambient) * vec3(material.albedo);
	vec3 diffuse = vec3(light.diffuse) * (diff * vec3(texture(diffuse_map, TexCoord)));
	vec3 specular = vec3(light.specular) * (spec * vec3(texture(specular_map, TexCoord)));

	float distance = length(vec3(light.position) - crntPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}