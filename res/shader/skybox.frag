#version 460 core
out vec4 frag_color;
in vec3 box_coord;

uniform samplerCube sky_box;

void main()
{    
    frag_color = vec4(1,1,1,1);
}