#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_EntityID;

layout(std140, binding = 6) uniform Camera
{
	mat4 u_ViewProjection;
	vec4 u_camPos;
};

layout(std140, binding = 7) uniform Model {
	mat4 u_Model;
};

layout (location = 0) out vec2 TexCoord;
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec3 Pos;
layout (location = 3) out vec4 camPos;
layout (location = 4) out flat int v_EntityID;

void main()
{
	v_EntityID = a_EntityID;
	TexCoord = a_TexCoord;
	mat3 normal_matrix = mat3(u_Model);
	normal_matrix = inverse(normal_matrix);
	normal_matrix = transpose(normal_matrix);
	Normal = normalize(normal_matrix * a_Normal);
	Pos = vec3(u_Model * vec4(a_Position, 1.0));
	camPos = u_camPos;
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);

}

