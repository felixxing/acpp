#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include "gls.hpp"

struct PtLight
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 color = {1.0f, 1.0f, 1.0f};

    float strength = 5.0f;
    float constant = 1.0f;
    float linear = 0.09;
    float quadratic = 0.032;
};

struct DirLight
{
    glm::vec3 direction = {0.0f, -1.0f, 0.0f};
    glm::vec3 color = {1.0f, 1.0f, 1.0f};

    float strength = 1.0f;
};

struct LightBlock
{
    uint32_t pt_lights_count = 0;
    uint32_t dir_lights_count = 0;

    PtLight pt_lights[500];
    DirLight dir_lights[10];
};

#endif // LIGHTS_HPP
