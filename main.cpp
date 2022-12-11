#include <iostream>
#include <thread>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "gls.hpp"
#include "glfw.hpp"

#include "shader.hpp"
#include "fbo.hpp"
#include "model.hpp"
#include "buffers.hpp"
#include "camera.hpp"
#include "lights.hpp"

int main(int argc, char** argv)
{
    GLWindow glfw(1920, 1080);
    glfw.set_hint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfw.set_hint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfw.set_hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfw.set_hint(GLFW_RESIZABLE, GLFW_FALSE);
    glfw.set_hint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfw.load("deffer shading");

    Texture default_tex;
    default_tex.load("res/textures/default_texture.png");
    Texture::default_texture = &default_tex;

    FrameBuff fbo(1920, 1080);
    for (int i = 0; i < 5; i++)
    {
        fbo.attachment = GL_COLOR_ATTACHMENT0 + i;
        fbo.attach_texture();
    }
    fbo.load_draws(
        {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4});
    fbo.attach_rbo(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);

    ScreenRect fbo_screen;

    Shader fbo_shader;
    fbo_shader.atatch_module(GL_VERTEX_SHADER, "res/shader/fbo.vert");
    fbo_shader.atatch_module(GL_FRAGMENT_SHADER, "res/shader/fbo.frag");
    fbo_shader.link();

    Shader mmm_shader;
    mmm_shader.atatch_module(GL_VERTEX_SHADER, "res/shader/render.vert");
    mmm_shader.atatch_module(GL_FRAGMENT_SHADER, "res/shader/render.frag");
    mmm_shader.link();

    Shader light_pass;
    light_pass.atatch_module(GL_VERTEX_SHADER, "res/shader/light.vert");
    light_pass.atatch_module(GL_FRAGMENT_SHADER, "res/shader/light.frag");
    light_pass.link();

    Model<20> mmm2("res/model/cube/cube.obj");
    mmm2.ins_count = 3;
    mmm2.ins_matrix[0] = glm::mat4(1.0f);
    mmm2.ins_matrix[0] = glm::translate(mmm2.ins_matrix[0], {3, 0.5, 0.0});
    mmm2.ins_matrix[1] = glm::mat4(1.0f);
    mmm2.ins_matrix[1] = glm::translate(mmm2.ins_matrix[1], {8, 2.0, 8.0});
    mmm2.ins_matrix[2] = glm::mat4(1.0f);
    mmm2.ins_matrix[2] = glm::scale(mmm2.ins_matrix[2], {20, 0.1, 20});

    Camera camera;
    camera.x = 0;
    camera.y = 0;
    camera.w = 1920;
    camera.h = 1080;
    camera.position = {-2, 0, 0};
    camera.update();

    UniformBuff camera_ubo(1, 2 * sizeof(glm::mat4));
    camera_ubo.add_data<glm::mat4>(camera.view_gl());
    camera_ubo.add_data<glm::mat4>(camera.proj_gl());

    LightBlock lb;
    lb.dir_lights_count = 1;
    lb.dir_lights[0].strength = 2;
    lb.dir_lights[0].direction = {-10.0f, -10.0, -10.0f};

    ShaderStorage light_ssbo(sizeof(lb), 2, &lb);

    double m_x = 0;
    double m_y = 0;
    glfwGetCursorPos(glfw.window, &m_x, &m_y);
    double m_x_p = m_x;
    double m_y_p = m_y;

    // shadow works
    const int shadow_w = 8102;
    const int shadow_h = 8102;
    FrameBuff depth_fbo(shadow_w, shadow_h);
    FrameBuff::attachment = GL_DEPTH_ATTACHMENT;
    FrameBuff::tex_format = GL_DEPTH_COMPONENT;
    FrameBuff::internal_format = GL_DEPTH_COMPONENT32F;
    FrameBuff::wrap = GL_CLAMP_TO_BORDER;
    depth_fbo.attach_texture();
    bool val_dfbo = depth_fbo.validate();

    Shader depth_shader;
    depth_shader.atatch_module(GL_VERTEX_SHADER, "res/shader/shadow.vert");
    depth_shader.link();

    Timer frame_timer;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);  
    glfwSetInputMode(glfw.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    while (!glfwWindowShouldClose(glfw.window))
    {
        frame_timer.start();
        glfwPollEvents();
        glfwGetCursorPos(glfw.window, &m_x, &m_y);

        {
            float speed = 15.0f * frame_timer.duration_s;
            if (camera.pitch > 79.0f)
            {
                camera.pitch = 79.0f;
            }
            else if (camera.pitch < -79.0f)
            {
                camera.pitch = -79.0f;
            }
            else
            {
                camera.pitch -= static_cast<float>((m_y - m_y_p) * speed);
            }

            camera.yaw -= static_cast<float>((m_x - m_x_p) * speed);

            m_x_p = m_x;
            m_y_p = m_y;
        }

        {
            float move_speed = 40.0f * frame_timer.duration_s;
            if (glfwGetKey(glfw.window, GLFW_KEY_W) == GLFW_PRESS)
            {
                camera.position += move_speed * camera.front;
            }
            if (glfwGetKey(glfw.window, GLFW_KEY_S) == GLFW_PRESS)
            {
                camera.position -= (move_speed * camera.front);
            }
            if (glfwGetKey(glfw.window, GLFW_KEY_A) == GLFW_PRESS)
            {
                camera.position -= move_speed * camera.right();
            }
            if (glfwGetKey(glfw.window, GLFW_KEY_D) == GLFW_PRESS)
            {
                camera.position += move_speed * camera.right();
            }
            if (glfwGetKey(glfw.window, GLFW_KEY_SPACE) == GLFW_PRESS)
            {
                camera.position += move_speed * camera.up;
            }
            if (glfwGetKey(glfw.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            {
                camera.position -= move_speed * camera.up;
            }
            if (glfwGetKey(glfw.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(glfw.window, true);
            }
        }

        camera.update();
        camera_ubo.load();

        {
            glEnable(GL_DEPTH_TEST);

            depth_fbo.bind();
            glCullFace(GL_FRONT);
            glClear(GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, shadow_w, shadow_h);
            float near_plane = 1.0f, far_plane = 25.0f;
            glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
            glm::mat4 lightView = glm::lookAt(-lb.dir_lights[0].direction, //
                                              glm::vec3(0.0f, 0.0f, 0.0f), //
                                              glm::vec3(0.0f, 1.0f, 0.0f));
            depth_shader.use();
            glm::mat4 light_space = lightProjection * lightView;
            glUniformMatrix4fv(depth_shader.uniform("light_space"), 1, GL_FALSE, glm::value_ptr(light_space));
            mmm2.draw(depth_shader);
            depth_fbo.unbind();

            fbo.bind();
            glCullFace(GL_BACK);
            glClearColor(0.0, 0.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, fbo.w, fbo.h);
            mmm_shader.use();
            glUniformMatrix4fv(mmm_shader.uniform("light_space"), 1, GL_FALSE, glm::value_ptr(light_space));
            mmm2.draw(mmm_shader);
            fbo.unbind();

            glDisable(GL_DEPTH_TEST);
        }

        light_ssbo.load();
        light_pass.use();

        glUniform1i(light_pass.uniform("positions"), 0);
        glUniform1i(light_pass.uniform("normals"), 1);
        glUniform1i(light_pass.uniform("colors"), 2);
        glUniform1i(light_pass.uniform("specs"), 3);
        glUniform1i(light_pass.uniform("proj_coords"), 4);
        glUniform1i(light_pass.uniform("shadows"), 5);
        glUniform3fv(light_pass.uniform("camera_pos"), 1, camera.position_gl());

        for (int i = 0; i < 5; i++)
        {
            fbo.textures[i]->bind(i);
        }
        depth_fbo.textures[0]->bind(5);
        fbo_screen.draw(fbo.w / 4, 0, 3 * fbo.w / 4, 3 * fbo.h / 4);
        // fbo_screen.draw(0, 0, fbo.w, fbo.h);
        for (int i = 0; i < 5; i++)
        {
            fbo.textures[i]->unbind();
        }
        depth_fbo.textures[0]->unbind();
        light_pass.unuse();

        fbo_shader.use();
        for (int i = 0; i < 4; i++)
        {
            fbo.textures[i]->bind();
            fbo_screen.draw(0, i * fbo.h / 4, fbo.w / 4, fbo.h / 4);
            fbo.textures[i]->unbind();
        }
        for (int i = 0; i < 1; i++)
        {
            fbo.textures[4 + i]->bind();
            fbo_screen.draw(i + 1 * fbo.w / 4, 3 * fbo.h / 4, fbo.w / 4, fbo.h / 4);
            fbo.textures[4 + i]->unbind();
        }
        fbo_shader.unuse();

        glfwSwapBuffers(glfw.window);
        frame_timer.finish();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}