#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoord;
layout(location = 4) in float a_TexIndex;
layout(location = 5) in float a_TilingFactor;
layout(location = 6) in int a_EntityID;
layout(location = 7) in int a_Material_Index;

layout(std140, binding = 2) uniform Camera
{
	mat4 u_ViewProjection;
	vec4 u_camPos;
};

layout (location = 0) out vec4 Color;
layout (location = 1) out vec2 TexCoord;
layout (location = 2) out float TilingFactor;
layout (location = 3) out flat float v_tex_index;
layout (location = 4) out flat int v_EntityID;
layout (location = 5) out vec3 Normal;
layout (location = 6) out vec3 Pos;
layout (location = 7) out flat int material_index;
layout (location = 8) out vec4 camPos;

void main()
{
	Color = a_Color;
	TexCoord = a_TexCoord;
	TilingFactor = a_TilingFactor;
	v_tex_index = a_TexIndex;
	v_EntityID = a_EntityID;
	Normal = a_Normal;
	Pos = a_Position;
	material_index = a_Material_Index;
	camPos = u_camPos;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
