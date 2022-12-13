#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normals;
layout(location = 2) in vec3 uvs;
layout(location = 3) in vec4 colors;
layout(location = 4) in mat4 model;

layout(std140, binding = 1) uniform cc_mat
{
    mat4 view;
    mat4 proj;
};

out VS_OUT
{
    vec3 frag_pos;
    vec3 normals;
    vec3 uvs;
    vec4 colors;
}
vs_out;

uniform mat4 light_space;

void main()
{
    vs_out.frag_pos = vec3(model * vec4(position, 1.0));
    vs_out.normals = mat3(transpose(inverse(model))) * normals;
    vs_out.uvs = uvs;
    vs_out.colors = colors;

    gl_Position = proj * view * vec4(vs_out.frag_pos, 1.0);
}