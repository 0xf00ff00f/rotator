#version 300 es

precision highp float;

uniform sampler2D baseColorTexture;

in vec2 vs_texcoord;
in vec4 vs_color;

out vec4 fragColor;

void main(void)
{
    float alpha = texture(baseColorTexture, vs_texcoord).r;
    vec4 color = vs_color;
    color.a *= alpha;
    fragColor = color;
}
