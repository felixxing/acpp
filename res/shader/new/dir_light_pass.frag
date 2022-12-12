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
uniform sampler2D shadow;

uniform vec3 camera_pos;
uniform mat4 light_space;

uniform vec3 light_dir = vec3(-1, -1, 0);
uniform vec3 light_color = vec3(1, 1, 1);
uniform float strength = 5;

void main()
{
    // set up data
    vec3 frag_pos = texture(positions, pt_light_fs_in.uv).rgb;
    vec3 normal = normalize(texture(normals, pt_light_fs_in.uv).rgb);
    vec3 frag_color = texture(colors, pt_light_fs_in.uv).rgb;
    vec3 frag_spec = texture(specs, pt_light_fs_in.uv).rgb;
    vec3 view_dir = normalize(camera_pos - frag_pos);

    // calculate light
    vec3 light_dir = -normalize(light_dir);

    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(normal, reflect_dir), 0.0), 32.0);

    vec3 ambient = frag_color * 0.1;
    vec3 diffuse = max(dot(normal, light_dir), 0.0) * frag_color;
    vec3 specular = spec * frag_spec;

    // calculate shadow
    vec4 light_space_pos = light_space * vec4(frag_pos, 1.0);
    vec3 proj_coord = light_space_pos.xyz / light_space_pos.w;
    proj_coord = proj_coord * 0.5 + 0.5;

    float closet_depth = texture(shadow, proj_coord.xy).r;
    float current_depth = proj_coord.z;
    float shadow_amout = current_depth < closet_depth ? 1.0 : 0.0;

    result = vec4(light_color * strength * (shadow_amout * (diffuse + specular) + ambient), 1.0);
}