#version 460 core

out vec4 FragColor;

in vec2 uv_;

uniform sampler2D screen;
uniform float gamma = 2.2;
uniform float exposure = 0.1;

void main()
{
    vec3 hdr_color = texture(screen, uv_).xyz;
    vec3 mapped = vec3(1.0) - exp(-hdr_color * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}