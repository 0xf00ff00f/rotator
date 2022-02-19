#version 300 es

precision highp float;

out vec4 fragColor;

in vec2 vs_texCoord;

void main(void)
{
    float edgeWidth = 0.03;
    float blur = 0.01;
    vec2 edgeLow = smoothstep(vec2(edgeWidth), vec2(edgeWidth + blur), vs_texCoord);
    vec2 edgeHigh = smoothstep(vec2(1.0) - vec2(edgeWidth), vec2(1.0) - vec2(edgeWidth + blur), vs_texCoord);
    float edge = edgeLow.x * edgeLow.y * edgeHigh.x * edgeHigh.y;
    fragColor = vec4(vec3(edge), 1.0);
}
