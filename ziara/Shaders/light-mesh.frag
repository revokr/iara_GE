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

vec3 getNormalFromMap();
vec3 calc_point_light(Material material,PointLight light, vec3 normal, vec3 crntPos, vec3 viewDir);
vec3 calc_dir_light(Material material,DirLight light, vec3 normal);
float shadowCalculation();

void main() {

	//vec3 norm = getNormalFromMap();

	vec3 N = normalize(Normal);
	mat3 TBN = mat3(T, B, N);

	vec3 normal = texture(normal_map, TexCoord).xyz * 2.0 - 1.0;
	normal = normalize(TBN * normal);

	vec3 viewDir = normalize(vec3(camPos) - Pos);
	vec3 result;

	if (skyLight.activeSkyLight == true) {
		result = calc_dir_light(material, skyLight, normal);
	} else {
		result = vec3(0.0);
	}
	for (int i = 0; i < nrLights; i++) {
		result += calc_point_light(material, lights[i], normal, Pos, viewDir);
	}

	/// GAMMA CORRECTION ???
	//float gamma = 2.2;
	//result = pow(result, vec3(1.0/gamma));

	color = vec4(result, 1.0);
	entityID = v_EntityID;
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

vec3 calc_dir_light(Material material, DirLight dlight, vec3 normal) {
	vec3 viewDir = normalize(vec3(camPos) - Pos);
	vec3 dir = vec3(dlight.direction);
	vec3 lightDir = normalize(-dir);

	float diff = max(dot(lightDir, normal), 0.0); 
	vec3 reflectionDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);

 	vec3 ambient = vec3(dlight.ambient) * vec3(material.albedo);
	vec3 diffuse = vec3(dlight.diffuse) * (diff * vec3(texture(diffuse_map, TexCoord)));
	vec3 specular = vec3(dlight.specular) * (spec * vec3(texture(specular_map, TexCoord)));

	float shadow = shadowCalculation();

	return (ambient + (1.0 - shadow + 0.2) * (diffuse + specular));
}

vec3 calc_point_light(Material material,PointLight light, vec3 normal, vec3 crntPos, vec3 viewDir) {
	vec3 lightDir = normalize(vec3(light.position) - crntPos);

	float diff = max(dot(lightDir, normal), 0.0); 
	vec3 reflectionDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

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