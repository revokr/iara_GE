#version 450 core
#define NR_POINT_LIGHTS 3
#define MAX_MATERIALS 10
#define MAX_LIGHTS 10

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;


layout (location = 0) in vec4 Color;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in float TilingFactor;
layout (location = 3) in flat float v_tex_index;
layout (location = 4) in flat int v_entityID;
layout (location = 5) in vec3 Normal;
layout (location = 6) in vec3 Pos;
layout (location = 7) in flat int material_index;
layout (location = 8) in vec4 camPos;

layout (binding = 0) uniform sampler2D u_Textures[32];

struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
	float padding1;
	vec2 padding2;
};

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

layout (std140, binding = 3) uniform MaterialUBO {
	Material materials[MAX_MATERIALS];
};

layout (std140, binding = 4) uniform PointLightsUBO {
	PointLight lights[MAX_LIGHTS];
	int nrLights;
};

layout (std140, binding = 5) uniform DirLightUBO {
	DirLight skyLight;
};

vec3 calc_point_light(Material material,PointLight light, vec3 normal, vec3 crntPos, vec3 viewDir);
vec3 calc_dir_light(Material material,DirLight light);

void main() {
	vec4 texColor = Color;

	if (material_index >= 0) {
		Material mat = materials[material_index];
		vec3 norm = normalize(Normal);
		vec3 viewDir = normalize(vec3(camPos) - Pos);
		vec3 result = calc_dir_light(mat, skyLight);
		for (int i = 0; i < nrLights; i++) {
			result += calc_point_light(mat, lights[i], norm, Pos, viewDir);
		}
		color = vec4(result, 1.0);
	} else {	
		color = Color;
	}

	color2 = v_entityID; 
}

vec3 calc_dir_light(Material material, DirLight dlight) {
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(vec3(camPos) - Pos);
	vec3 dir = vec3(dlight.direction);
	vec3 lightDir = normalize(-dir);

	float diff = max(dot(lightDir, normal), 0.0); 
	vec3 reflectionDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);

 	vec3 ambient = vec3(dlight.ambient) * vec3(material.ambient);
	vec3 diffuse = vec3(dlight.diffuse) * (diff * vec3(material.diffuse));
	vec3 specular = vec3(dlight.specular) * (spec * vec3(material.specular));

	return (ambient + diffuse + specular);
}

vec3 calc_point_light(Material material,PointLight light, vec3 normal, vec3 crntPos, vec3 viewDir) {
	vec3 lightDir = normalize(vec3(light.position) - crntPos);

	float diff = max(dot(lightDir, normal), 0.0); 
	vec3 reflectionDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);

 	vec3 ambient = vec3(light.ambient) * vec3(material.ambient);
	vec3 diffuse = vec3(light.diffuse) * (diff * vec3(material.diffuse));
	vec3 specular = vec3(light.specular) * (spec * vec3(material.specular));

	float distance = length(vec3(light.position) - crntPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}