#version 300 es

precision highp float;

uniform vec4 mixColor;

out vec4 fragColor;

in vec2 vs_texCoord;

void main(void)
{
    float edgeWidth = 0.04;
    float blur = 0.02;
    vec2 edgeLow = smoothstep(vec2(edgeWidth), vec2(edgeWidth + blur), vs_texCoord);
    vec2 edgeHigh = smoothstep(vec2(1.0) - vec2(edgeWidth), vec2(1.0) - vec2(edgeWidth + blur), vs_texCoord);
    float edge = edgeLow.x * edgeLow.y * edgeHigh.x * edgeHigh.y;
    fragColor = vec4(mix(vec3(edge), mixColor.rgb, mixColor.a), 1.0);
}
