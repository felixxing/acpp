#version 450 core

layout(location = 0) in vec3 position;
layout(location = 4) in mat4 model;

uniform mat4 light_space;

void main()
{
    gl_Position = light_space * model * vec4(position, 1.0);
}