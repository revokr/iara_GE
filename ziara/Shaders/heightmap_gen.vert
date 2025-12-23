#version 450 core

layout(location = 0) in vec2 inUV;   // grid coords [0,1]
layout(location = 0) out vec2 vUV;

layout(binding = 0) uniform sampler2D uDepth;

layout(std140, binding = 26) uniform buf {
    mat4 uMVP;
};


void main() {

    vec2 uSize   = vec2(848.0, 480.0);
    vec2 uCenter = vec2(424.0, 240.0);
    vec2 uFocal  = vec2(416.0, 416.0); // estimated

    vUV = inUV;

    // Convert [0,1] UV ? pixel coords
    vec2 pixel = inUV * uSize;

    // Sample depth (meters ideally)
    float d = texture(uDepth, inUV).r;

    // Back-project
    float X = (pixel.x - uCenter.x) * d / uFocal.x;
    float Y = (pixel.y - uCenter.y) * d / uFocal.y;
    float Z = d;


    gl_Position = uMVP * vec4(X, Y, Z, 1.0);
}