#version 300 es

layout(location=0) in vec2 position;
layout(location=1) in vec2 texcoord;
layout(location=2) in vec4 color;

uniform mat4 mvp;

out vec2 vs_texcoord;
out vec4 vs_color;

void main(void)
{
    vs_texcoord = texcoord;
    vs_color = color;
    gl_Position = mvp * vec4(position, 0, 1);
}
