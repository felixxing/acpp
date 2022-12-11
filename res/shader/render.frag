#version 460 core

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;
layout(location = 2) out vec4 colors;
layout(location = 3) out vec4 specs;

// ins
in VS_OUT
{
    vec3 frag_pos;
    vec3 normals;
    vec3 uvs;
    vec4 colors;
}
fs_in;

uniform sampler2D diff_map;
uniform sampler2D spec_map;
uniform sampler2D ambi_map;
uniform sampler2D emis_map;
uniform sampler2D opac_map;

void main()
{
    if (texture(opac_map, fs_in.uvs.xy).r <= 0)
    {
        discard;
    }

    position = vec4(fs_in.frag_pos, 1.0);

    normal = vec4(fs_in.normals, 1.0);

    colors = fs_in.colors * texture(diff_map, fs_in.uvs.xy);

    specs = fs_in.colors * texture(spec_map, fs_in.uvs.xy);
}