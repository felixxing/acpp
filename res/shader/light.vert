#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

out LIGHT_VS_OUT
{
    vec2 uv;
} light_vs_out;

void main()
{
    gl_Position = vec4(position.xy, 1.0, 1.0);
    light_vs_out.uv = uv;
}