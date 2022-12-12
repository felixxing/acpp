#version 460 core

layout(location = 0) out vec4 result;

in LIGHT_VS_OUT
{
    vec2 uv;
}
pt_light_fs_in;

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D colors;
uniform sampler2D specs;
uniform vec3 camera_pos;

uniform vec3 position;
uniform vec3 color;

uniform float strength;
uniform float constant;
uniform float linear;
uniform float quadratic;

void main()
{
    vec3 frag_pos = texture(positions, pt_light_fs_in.uv).rgb;
    vec3 normal = normalize(texture(normals, pt_light_fs_in.uv).rgb);
    vec3 frag_color = texture(colors, pt_light_fs_in.uv).rgb;
    vec3 frag_spec = texture(specs, pt_light_fs_in.uv).rgb;
    vec3 view_dir = normalize(camera_pos - frag_pos);

    

    result = texture(positions, pt_light_fs_in.uv);
}