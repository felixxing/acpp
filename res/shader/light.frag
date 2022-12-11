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

uniform sampler2D positions;
uniform sampler2D normals;
uniform sampler2D colors;
uniform sampler2D specs;
uniform sampler2D shadows;

uniform vec3 camera_pos;
uniform mat4 light_space;

struct PtLight
{
    vec3 position;
    vec3 color;

    float strength;
    float constant;
    float linear;
    float quadratic;
};

struct DirLight
{
    vec3 direction;
    vec3 color;

    float strength;
};

vec3 get_pt_light(vec3 view_dir, vec3 fpos, vec3 fnormal, vec3 fcolor, vec3 fspec, PtLight pt_light);
vec3 get_dir_light(vec3 view_dir, vec3 fpos, vec3 fnormal, vec3 fcolor, vec3 fspec, DirLight dir_light);
float get_shadow(vec3 proj_cod, float bias);

void main()
{
    vec3 frag_pos = texture(positions, light_fs_in.uv).rgb;
    vec3 normal = normalize(texture(normals, light_fs_in.uv).rgb);
    vec3 frag_color = texture(colors, light_fs_in.uv).rgb;
    vec3 frag_spec = texture(specs, light_fs_in.uv).rgb;

    vec3 view_dir = normalize(camera_pos - frag_pos);

    vec3 light_result = vec3(0);
    for (int i = 0; i < pt_lights_count; i++)
    {
        PtLight ppt_light;
        ppt_light.position = vec3(pt_lights[i][0], pt_lights[i][1], pt_lights[i][2]);
        ppt_light.color = vec3(pt_lights[i][3], pt_lights[i][4], pt_lights[i][5]);
        ppt_light.strength = pt_lights[i][6];
        ppt_light.constant = pt_lights[i][7];
        ppt_light.linear = pt_lights[i][8];
        ppt_light.quadratic = pt_lights[i][9];

        light_result += get_pt_light(view_dir, frag_pos, normal, frag_color, frag_spec, ppt_light);
    }

    for (int i = 0; i < dir_light_count; i++)
    {
        DirLight ddir_light;
        ddir_light.direction = vec3(dir_lights[i][0], dir_lights[i][1], dir_lights[i][2]);
        ddir_light.color = vec3(dir_lights[i][3], dir_lights[i][4], dir_lights[i][5]);
        ddir_light.strength = dir_lights[i][6];

        light_result += get_dir_light(view_dir, frag_pos, normal, frag_color, frag_spec, ddir_light);
    }

    result = vec4(light_result, 1.0);
}

vec3 get_pt_light(vec3 view_dir, vec3 fpos, vec3 fnormal, vec3 fcolor, vec3 fspec, PtLight pt_light)
{
    vec3 light_dir = normalize(pt_light.position - fpos);

    vec3 half_way = normalize(light_dir + view_dir);
    float spec = pow(max(dot(fnormal, half_way), 0.0), 32.0);

    float dist = length(pt_light.position - fpos);
    float attenuation = 1.0 / (pt_light.constant + pt_light.linear * dist + pt_light.quadratic * dist * dist);

    vec3 ambient = fcolor * 0.1;
    vec3 diffuse = max(dot(fnormal, light_dir), 0.0) * fcolor;
    vec3 specular = spec * fspec;

    return pt_light.color * pt_light.strength * attenuation * (diffuse + ambient + specular);
}

vec3 get_dir_light(vec3 view_dir, vec3 fpos, vec3 fnormal, vec3 fcolor, vec3 fspec, DirLight dir_light)
{
    vec3 light_dir = -normalize(dir_light.direction);

    vec3 reflectDir = reflect(-light_dir, fnormal);
    float spec = pow(max(dot(fnormal, reflectDir), 0.0), 32.0);

    vec3 ambient = fcolor * 0.1;
    vec3 diffuse = max(dot(fnormal, light_dir), 0.0) * fcolor;
    vec3 specular = spec * fspec;

    vec4 light_coord = light_space * vec4(fpos, 1.0);
    vec3 proj_coord = light_coord.xyz / light_coord.w;
    float bias = max(0.001 * (1.0 - dot(fnormal, light_dir)), 0.0004);
    float shadow = get_shadow(proj_coord, bias);

    return 0.5 * dir_light.color * dir_light.strength * ((1 - shadow) * (diffuse + specular) + ambient);
}

float get_shadow(vec3 proj_cod, float bias)
{
    proj_cod = proj_cod * 0.5 + 0.5;

    float closestDepth = texture(shadows, proj_cod.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = proj_cod.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    if (proj_cod.z >= 1.0)
    {
        shadow = 0.0;
    }

    return shadow;
}