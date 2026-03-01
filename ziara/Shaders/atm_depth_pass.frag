#version 440

layout(location = 0) in vec3 view_ray;

layout(std140, binding = 23) uniform ViewData {
	mat4 model_from_view;
	mat4 view_from_clip;
	mat4 view_from_model;
	mat4 clip_from_view;
};

layout(std140, binding = 24) uniform Data {
    vec4 camera_;
    vec4 white_point_;
    vec4 earth_center_;
    vec4 sun_direction_;
    vec4 sun_size_;
    vec4 viewport_size;
    float exposure;
};

void main() {
    const float radius = 6360.0;
    
    vec3 ray_origin = camera_.xyz;
    vec3 ray_dir = normalize(view_ray);

    vec3 center = earth_center_.xyz;

    vec3 oc = ray_origin - center;

    float b = dot(oc, ray_dir);
    float c = dot(oc, oc) - radius * radius;
    
    float delta = b * b - c;
    if (delta < 0.0) discard;

    delta = sqrt(delta);

    float t1 = (-b - delta);
    float t2 = (-b + delta);

    if (t2 < 0.0) discard;

    float t = (t1 > 0.0) ? t1 : t2;

    vec3 x_world = ray_origin + t * ray_dir;

    vec4 x_view = view_from_model * vec4(x_world, 1.0);
    vec4 x_clip = clip_from_view * x_view;

    float ndc_depth = x_clip.z / x_clip.w;
    float depth = ndc_depth * 0.5 + 0.5;
    gl_FragDepth = clamp(depth, 0.0, 1.0);
}