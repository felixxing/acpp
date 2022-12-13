#ifndef PT_LIGHT_HPP
#define PT_LIGHT_HPP

#include <functional>

#include "fbo.hpp"
#include "shader.hpp"

class PtLight
{
  private:
    Framebuffer light_space_depth;
    Texture2D* shadow_map = nullptr;

    glm::mat4 light_proj;
    glm::mat4 light_space[6];

    int done_shadow = 0;

  public:
    static Shader* shadow_pass_shader;
    static Shader* color_pass_shader;
    bool validation = false;

    float near = 1.0f;
    float far = 500.0f;

    glm::vec3 light_pos = {0, 0, 0};
    glm::vec3 light_color = {1, 1, 1};
    float strength = 100;
    float constant = 1;
    float linear = 0.09;
    float quadratic = 0.032;

    PtLight(int shadow_w, int shadow_h)
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
        depth_create_info.target_type = GL_TEXTURE_CUBE_MAP;

        light_space_depth.attach_texture(depth_create_info, GL_DEPTH_ATTACHMENT);
        validation = light_space_depth.validate();

        shadow_map = light_space_depth.textures[0];
    }

    void shadow_pass(std::function<void()> draw_calls)
    {
        done_shadow = 1;
        float aspect = static_cast<float>(light_space_depth.w) / static_cast<float>(light_space_depth.h);
        light_proj = glm::perspective(glm::radians(90.0f), //
                                      aspect,              //
                                      near, far);
        // xs
        light_space[0] = light_proj * glm::lookAt(light_pos,                            //
                                                  light_pos + glm::vec3(1.0, 0.0, 0.0), //
                                                  glm::vec3(0.0, -1.0, 0.0));
        light_space[1] = light_proj * glm::lookAt(light_pos,                             //
                                                  light_pos + glm::vec3(-1.0, 0.0, 0.0), //
                                                  glm::vec3(0.0, -1.0, 0.0));
        // ys
        light_space[2] = light_proj * glm::lookAt(light_pos,                            //
                                                  light_pos + glm::vec3(0.0, 1.0, 0.0), //
                                                  glm::vec3(0.0, 0.0, 1.0));
        light_space[3] = light_proj * glm::lookAt(light_pos,                             //
                                                  light_pos + glm::vec3(0.0, -1.0, 0.0), //
                                                  glm::vec3(0.0, 0.0, -1.0));
        // zs
        light_space[4] = light_proj * glm::lookAt(light_pos,                            //
                                                  light_pos + glm::vec3(0.0, 0.0, 1.0), //
                                                  glm::vec3(0.0, -1.0, 0.0));
        light_space[5] = light_proj * glm::lookAt(light_pos,                             //
                                                  light_pos + glm::vec3(0.0, 0.0, -1.0), //
                                                  glm::vec3(0.0, -1.0, 0.0));

        glEnable(GL_DEPTH_TEST);
        light_space_depth.bind();
        glCullFace(GL_FRONT);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, light_space_depth.w, light_space_depth.h);

        shadow_pass_shader->use();
        glUniformMatrix4fv(shadow_pass_shader->uniform("shadowMatrices"), 6, GL_FALSE, //
                           glm::value_ptr(light_space[0]));
        glUniform3fv(shadow_pass_shader->uniform("lightPos"), 1, glm::value_ptr(light_pos));
        glUniform1f(shadow_pass_shader->uniform("far_plane"), far);

        draw_calls();

        shadow_pass_shader->unuse();
        light_space_depth.unbind();
        glDisable(GL_DEPTH_TEST);
    }

    void color_pass(std::function<void()> draw_calls)
    {
        color_pass_shader->use();

        // set texture uniforms
        glUniform3fv(color_pass_shader->uniform("light_pos"), 1, glm::value_ptr(light_pos));
        glUniform3fv(color_pass_shader->uniform("light_color"), 1, glm::value_ptr(light_color));
        glUniform1f(color_pass_shader->uniform("strength"), strength);
        glUniform1f(color_pass_shader->uniform("constant"), constant);
        glUniform1f(color_pass_shader->uniform("linear"), linear);
        glUniform1f(color_pass_shader->uniform("quadratic"), quadratic);
        glUniform1i(color_pass_shader->uniform("done_shadow"), done_shadow);

        glUniform1f(color_pass_shader->uniform("far_plane"), far);
        shadow_map->bind(SHADOW);

        draw_calls();

        shadow_map->unbind();
        color_pass_shader->unuse();
    }

    static void init()
    {
        static bool pt_light_ready = false;

        if (!pt_light_ready)
        {
            glEnable(GL_CULL_FACE);
            PtLight::shadow_pass_shader = new Shader;
            PtLight::color_pass_shader = new Shader;

            PtLight::shadow_pass_shader->atatch_module(GL_VERTEX_SHADER, "res/shader/pt_shadow.vert");
            PtLight::shadow_pass_shader->atatch_module(GL_GEOMETRY_SHADER, "res/shader/pt_shadow.geom");
            PtLight::shadow_pass_shader->atatch_module(GL_FRAGMENT_SHADER, "res/shader/pt_shadow.frag");
            PtLight::shadow_pass_shader->link();

            PtLight::color_pass_shader->atatch_module(GL_VERTEX_SHADER, "res/shader/light.vert");
            PtLight::color_pass_shader->atatch_module(GL_FRAGMENT_SHADER, "res/shader/pt_light_pass.frag");
            PtLight::color_pass_shader->link();

            PtLight::color_pass_shader->use();
            glUniform1i(PtLight::color_pass_shader->uniform("positions"), POSITION);
            glUniform1i(PtLight::color_pass_shader->uniform("normals"), NORMAL);
            glUniform1i(PtLight::color_pass_shader->uniform("colors"), COLOR);
            glUniform1i(PtLight::color_pass_shader->uniform("specs"), SPECULAR);
            glUniform1i(PtLight::color_pass_shader->uniform("shadow"), SHADOW);
            PtLight::color_pass_shader->unuse();

            pt_light_ready = true;
        }
    }

    Texture2D* get_shadow_map() const
    {
        return shadow_map;
    }
};

inline Shader* PtLight::shadow_pass_shader = nullptr;
inline Shader* PtLight::color_pass_shader = nullptr;

#endif // PT_LIGHT_HPP
