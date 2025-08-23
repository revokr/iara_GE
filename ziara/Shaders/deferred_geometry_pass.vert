#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;
layout(location = 5) in int a_EntityID;

layout(location = 0) out vec3 Pos;
layout(location = 1) out vec2 TexCoord;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 T;
layout(location = 4) out vec3 B;
layout(location = 5) out flat int v_EntityID;

layout(std140, binding = 6) uniform Camera {
	mat4 u_ViewProjection;
	vec4 u_camPos;
};

layout(std140, binding = 7) uniform Model {
	mat4 u_Model;
};

void main()
{
	Pos = vec3(u_Model * vec4(a_Position, 1.0));
	TexCoord = a_TexCoord;

	mat3 normal_matrix = mat3(u_Model);
	normal_matrix = inverse(normal_matrix);
	normal_matrix = transpose(normal_matrix);
	Normal = normalize(normal_matrix * a_Normal);

	T = normalize(normal_matrix * a_Tangent);
	B = normalize(normal_matrix * a_Bitangent);

	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(Pos, 1.0);
}

