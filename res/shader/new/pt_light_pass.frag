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
uniform samplerCube shadow;
uniform vec3 camera_pos;
uniform float far_plane;

uniform vec3 light_pos = vec3(2, 2, 2);
uniform vec3 light_color = vec3(1, 1, 1);

uniform float strength = 100;
uniform float constant = 1;
uniform float linear = 0.09;
uniform float quadratic = 0.032;

void main()
{
    vec3 frag_pos = texture(positions, pt_light_fs_in.uv).rgb;
    vec3 normal = normalize(texture(normals, pt_light_fs_in.uv).rgb);
    vec3 frag_color = texture(colors, pt_light_fs_in.uv).rgb;
    vec3 frag_spec = texture(specs, pt_light_fs_in.uv).rgb;
    vec3 view_dir = normalize(camera_pos - frag_pos);

    vec3 light_dir = normalize(light_pos - frag_pos);

    vec3 half_way = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, half_way), 0.0), 32.0);

    float dist = length(light_pos - frag_pos);
    float attenuation = 1.0 / (constant + linear * dist + quadratic * dist * dist);

    vec3 ambient = frag_color * 0.1;
    vec3 diffuse = max(dot(normal, light_dir), 0.0) * frag_color;
    vec3 specular = spec * frag_spec;

    // calcualte depth
    vec3 frag_light = frag_pos - light_pos;
    float closet_depth = texture(shadow, frag_light).r;
    closet_depth *= far_plane;
    float current_depth = length(frag_light);
    float shadow_amout = current_depth < closet_depth ? 1.0 : 0.0;

    result = vec4(light_color * strength * attenuation * (shadow_amout * (specular + diffuse) + ambient), 1.0);
}