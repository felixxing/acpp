#version 450 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

out vec2 uv_;

void main()
{
    gl_Position = vec4(position.xy, 1.0, 1.0);
    uv_ = uv;
}