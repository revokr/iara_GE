#version 440

layout(location = 0) in vec3 vertex;

layout(location = 0) out vec3 view_ray;

layout(std140, binding = 23) uniform ViewData {
	mat4 model_from_view;
	mat4 view_from_clip;
};

void main() {
	gl_Position = vec4(vec2(vertex), 1.0, 1.0);

	vec4 view_pos = view_from_clip * vec4(vec2(vertex), 1.0, 1.0);

	view_pos.xyz /= view_pos.w;

	vec3 view_dir = normalize(view_pos.xyz);

	view_ray = (model_from_view * vec4(view_dir, 0.0)).xyz;
}