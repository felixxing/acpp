#version 460 core

layout(location = 0) out vec4 result;

in LIGHT_VS_OUT
{
    vec2 uv;
}
light_fs_in;

layout(std430, binding = 2) buffer light_data
{
    uint pt_lights_count;
    uint dir_light_count;

    float pt_lights[500][10];
    float dir_lights[10][7];
};

uniform sampler2D position;
uniform sampler2D normal;
uniform sampler2D colors;
uniform sampler2D specs;

uniform vec3 camera_pos;

void main()
{
    vec3 frag_pos = texture(position, light_fs_in.uv).rgb;
    vec3 normal = normalize(texture(normal, light_fs_in.uv).rgb);
    vec3 frag_color = texture(colors, light_fs_in.uv).rgb;
    vec3 frag_spec = texture(specs, light_fs_in.uv).rgb;

    vec3 light_result = vec3(0);
    for (int i = 0; i < pt_lights_count; i++)
    {
        vec3 light_pos = vec3(pt_lights[i][0], pt_lights[i][1], pt_lights[i][2]);
        vec3 light_color = vec3(pt_lights[i][3], pt_lights[i][4], pt_lights[i][5]);
        float light_strength = pt_lights[i][6];
        float light_constant = pt_lights[i][7];
        float light_linear = pt_lights[i][8];
        float light_quadratic = pt_lights[i][9];

        vec3 view_dir = normalize(camera_pos - frag_pos);
        vec3 light_dir = normalize(light_pos[i] - frag_pos);

        vec3 half_way = normalize(light_dir + view_dir);
        float spec = pow(max(dot(normal, half_way), 0.0), 32.0);

        float dist = length(light_pos[i] - frag_pos);
        float attenuation = 1.0 / (light_constant + light_linear * dist + light_quadratic * dist * dist);

        vec3 ambient = frag_color * 0.1 * light_color;
        vec3 diffuse = max(dot(normal, light_dir), 0.0) * frag_color * light_color;
        vec3 specular = spec * frag_spec * light_color;

        light_result += light_strength * attenuation * (diffuse + ambient + specular);
    }

    result = vec4(light_result, 1.0);
}