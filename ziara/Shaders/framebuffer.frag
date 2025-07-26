#version 330 core

out vec4 fragColor;
in vec2 texCoords;

uniform sampler2DMS screenTex;

void main()
{
    ivec2 texelCoord = ivec2(gl_FragCoord.xy);
    vec4 color = texelFetch(screenTex, texelCoord, 0); // Fetch a sample
    fragColor = color;
}  