#version 460 core

layout(location = 0) in vec3 position;
layout(location = 4) in mat4 model;

void main()
{
    gl_Position = model * vec4(position, 1.0);
}