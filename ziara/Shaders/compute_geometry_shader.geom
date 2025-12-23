#version 440

layout(triangles) in;

layout(triangle_strip, max_vertices = 3) out;

layout (std140, binding = 30) uniform layerubo {
    int layer;
};

void main() {
    gl_Position = gl_in[0].gl_Position;
    gl_Layer = layer;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    gl_Layer = layer;
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    gl_Layer = layer;
    EmitVertex();
    EndPrimitive();
}