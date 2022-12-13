#ifndef DIR_LIGHT_HPP
#define DIR_LIGHT_HPP

#include <functional>

#include "fbo.hpp"
#include "shader.hpp"

class DirLight
{
  private:
    Framebuffer light_space_depth;
    Texture2D* shadow_map = nullptr;

    glm::mat4 light_proj;
    glm::mat4 light_space;

    int done_shadow = 0;

  public:
    static Shader* shadow_pass_shader;
    static Shader* color_pass_shader;
    bool validation = false;

    float near = 1.0f;
    float far = 500.0f;

    float position_ratio = 200.0f;
    glm::vec3 direction = {-1, -1, 0};
    glm::vec3 color = {1, 1, 1};
    glm::vec4 rectangle = {-200.0f, 200.0f, -200.0f, 200.0f};

    float strength = 5.0f;

    DirLight(int shadow_w, int shadow_h)
        : light_space_depth(shadow_w, shadow_h)
    {
        init();

        Texture2DCreateInfo depth_create_info;
        depth_create_info.internal_format = GL_DEPTH_COMPONENT24;
        depth_create_info.wrap_s = GL_CLAMP_TO_EDGE;
        depth_create_info.wrap_r = GL_CLAMP_TO_EDGE;
        depth_create_info.wrap_t = GL_CLAMP_TO_EDGE;
        depth_create_info.min_filter = GL_NEAREST;
        depth_create_info.mag_filter = GL_NEAREST;
        depth_create_info.width = shadow_w;
        depth_create_info.height = shadow_h;
        depth_create_info.levels = 1;

        light_space_depth.attach_texture(depth_create_info, GL_DEPTH_ATTACHMENT);
        validation = light_space_depth.validate();
        shadow_map = light_space_depth.textures[0];
    }

    void shadow_pass(std::function<void()> draw_calls)
    {
        done_shadow = 1;
        light_proj = glm::ortho(rectangle.x, rectangle.y, rectangle.z, rectangle.w, near, far);
        light_space = light_proj * glm::lookAt(-position_ratio * direction, //
                                               glm::vec3(0.0f, 0.0f, 0.0f), //
                                               glm::vec3(0.0f, 1.0f, 0.0f));

        glEnable(GL_DEPTH_TEST);
        light_space_depth.bind();
        glCullFace(GL_FRONT);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, light_space_depth.w, light_space_depth.h);
        shadow_pass_shader->use();

        glUniformMatrix4fv(shadow_pass_shader->uniform("light_space"), 1, GL_FALSE, glm::value_ptr(light_space));
        draw_calls();

        shadow_pass_shader->unuse();
        light_space_depth.unbind();
        glDisable(GL_DEPTH_TEST);
    }

    void color_pass(std::function<void()> draw_calls)
    {
        color_pass_shader->use();
        glUniformMatrix4fv(color_pass_shader->uniform("light_space"), 1, GL_FALSE, glm::value_ptr(light_space));
        shadow_map->bind(SHADOW);

        glUniform3fv(color_pass_shader->uniform("light_dir"), 1, glm::value_ptr(direction));
        glUniform3fv(color_pass_shader->uniform("light_color"), 1, glm::value_ptr(color));
        glUniform1i(color_pass_shader->uniform("done_shadow"), done_shadow);
        glUniform1f(color_pass_shader->uniform("strength"), strength);

        draw_calls();

        shadow_map->unbind();
        color_pass_shader->unuse();
    }

    static void init()
    {
        static bool dir_light_ready = false;

        if (!dir_light_ready)
        {
            glEnable(GL_CULL_FACE);
            DirLight::shadow_pass_shader = new Shader;
            DirLight::color_pass_shader = new Shader;

            shadow_pass_shader->atatch_module(GL_VERTEX_SHADER, "res/shader/dir_shadow.vert");
            shadow_pass_shader->link();

            color_pass_shader->atatch_module(GL_VERTEX_SHADER, "res/shader/light.vert");
            color_pass_shader->atatch_module(GL_FRAGMENT_SHADER, "res/shader/dir_light_pass.frag");
            color_pass_shader->link();

            DirLight::color_pass_shader->use();
            glUniform1i(DirLight::color_pass_shader->uniform("positions"), POSITION);
            glUniform1i(DirLight::color_pass_shader->uniform("normals"), NORMAL);
            glUniform1i(DirLight::color_pass_shader->uniform("colors"), COLOR);
            glUniform1i(DirLight::color_pass_shader->uniform("specs"), SPECULAR);
            glUniform1i(DirLight::color_pass_shader->uniform("shadow"), SHADOW);
            DirLight::color_pass_shader->unuse();

            dir_light_ready = true;
        }
    }

    Texture2D* get_shadow_map() const
    {
        return shadow_map;
    }
};

inline Shader* DirLight::shadow_pass_shader = nullptr;
inline Shader* DirLight::color_pass_shader = nullptr;

#endif // DIR_LIGHT_HPP
