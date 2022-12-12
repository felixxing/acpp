#version 460 core

layout(location = 0) in vec3 position;

layout(std140, binding = 1) uniform cc_mat
{
    mat4 view;
    mat4 proj;
};
out vec3 box_coord;

void main()
{
    box_coord = position;
    gl_Position = proj * mat4(mat3(view)) * vec4(position, 1.0);
}