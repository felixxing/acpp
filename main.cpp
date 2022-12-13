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
#include "pt_light.hpp"
#include "dir_light.hpp"

int glmain()
{
    GLWindow glfw(1920, 1080);
    glfw.set_hint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfw.set_hint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfw.set_hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfw.set_hint(GLFW_RESIZABLE, GLFW_FALSE);
    glfw.set_hint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
    glfw.load("Renderer");

    Texture2D default_tex2d;
    Texture2DCreateInfo default_create_info;
    default_tex2d.create(default_create_info, "res/textures/white.png");
    Texture2D::default_texture = &default_tex2d;

    Framebuffer gbuffer(glfw.width, glfw.height);
    bool gbuffer_validation = false;
    auto create_gbuffer = [&]()
    {
        // position and normals
        Texture2DCreateInfo create_info;
        create_info.internal_format = GL_RGBA16F;
        create_info.levels = 1;
        create_info.min_filter = GL_LINEAR;
        create_info.width = gbuffer.w;
        create_info.height = gbuffer.h;
        gbuffer.attach_texture(create_info, GL_COLOR_ATTACHMENT0);
        gbuffer.attach_texture(create_info, GL_COLOR_ATTACHMENT1);
        // color
        create_info.internal_format = GL_RGBA8;
        gbuffer.attach_texture(create_info, GL_COLOR_ATTACHMENT2);
        // spec
        create_info.internal_format = GL_RGBA8;
        gbuffer.attach_texture(create_info, GL_COLOR_ATTACHMENT3);

        // depth
        create_info.internal_format = GL_DEPTH_COMPONENT32;
        create_info.wrap_s = GL_CLAMP_TO_BORDER;
        create_info.wrap_r = GL_CLAMP_TO_BORDER;
        create_info.wrap_t = GL_CLAMP_TO_BORDER;
        create_info.min_filter = GL_LINEAR;
        create_info.width = gbuffer.w;
        create_info.height = gbuffer.h;
        create_info.levels = 1;
        gbuffer.attach_texture(create_info, GL_DEPTH_ATTACHMENT);
        gbuffer.draw_buffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3});
        gbuffer_validation = gbuffer.validate();
    };
    create_gbuffer();

    Framebuffer light_pass_buffer(glfw.width, glfw.height);
    bool light_pass_buffer_validation = false;
    auto create_lighting_pass_buffer = [&]()
    {
        Texture2DCreateInfo create_info;
        create_info.internal_format = GL_RGBA16F;
        create_info.levels = 1;
        create_info.min_filter = GL_LINEAR;
        create_info.width = light_pass_buffer.w;
        create_info.height = light_pass_buffer.h;
        light_pass_buffer.attach_texture(create_info, GL_COLOR_ATTACHMENT0);

        light_pass_buffer_validation = light_pass_buffer.validate();
    };
    create_lighting_pass_buffer();

    ScreenRect screen;

    Shader screen_shader;
    Shader gbuffer_shader;
    auto init_shaders = [&]()
    {
        screen_shader.atatch_module(GL_VERTEX_SHADER, "res/shader/screen.vert");
        screen_shader.atatch_module(GL_FRAGMENT_SHADER, "res/shader/screen.frag");
        screen_shader.link();

        gbuffer_shader.atatch_module(GL_VERTEX_SHADER, "res/shader/gbuffer.vert");
        gbuffer_shader.atatch_module(GL_FRAGMENT_SHADER, "res/shader/gbuffer.frag");
        gbuffer_shader.link();
    };
    init_shaders();

    Model<20> sponza_model("res/model/sponza/sponza.obj");
    sponza_model.ins_count = 1;
    sponza_model.ins_matrix[0] = glm::scale(glm::mat4(1.0f), {0.1f, 0.1f, 0.1f});

    Model<5> cube("res/model/cube/cube.obj");
    cube.ins_count = 1;

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

    double m_x = 0;
    double m_y = 0;
    glfwGetCursorPos(glfw.window, &m_x, &m_y);
    double m_x_p = m_x;
    double m_y_p = m_y;

    Timer frame_timer;
    glfwSetInputMode(glfw.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    PtLight pt_light1(1024, 1024);
    pt_light1.light_pos = {0, 70, 0};

    DirLight dir_light1(10000, 10000);

    pt_light1.shadow_pass(
        [&]()
        {
            sponza_model.draw();
            cube.draw();
        });

    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
        cube.ins_matrix[0] = glm::translate(glm::mat4(1.0f), camera.position);

        auto draw_gbuffer = [&]()
        {
            glEnable(GL_DEPTH_TEST);
            gbuffer.bind();
            glCullFace(GL_BACK);
            glClearColor(0.0, 0.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, gbuffer.w, gbuffer.h);
            sponza_model.draw(gbuffer_shader);
            cube.draw(gbuffer_shader);
            gbuffer.unbind();
            glDisable(GL_DEPTH_TEST);

            dir_light1.shadow_pass(
                [&]()
                {
                    sponza_model.draw();
                    cube.draw();
                });
        };
        draw_gbuffer();

        auto ligthing_pass = [&]()
        {
            glCullFace(GL_BACK);
            gbuffer.textures[0]->bind(POSITION);
            gbuffer.textures[1]->bind(NORMAL);
            gbuffer.textures[2]->bind(COLOR);
            gbuffer.textures[3]->bind(SPECULAR);

            light_pass_buffer.bind();
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glClear(GL_COLOR_BUFFER_BIT);

            pt_light1.color_pass(
                [&]()
                {
                    glUniform3fv(PtLight::color_pass_shader->uniform("camera_pos"), 1, camera.position_gl());
                    screen.draw(0, 0, glfw.width, glfw.height);
                });

            dir_light1.color_pass(
                [&]()
                {
                    glUniform3fv(DirLight::color_pass_shader->uniform("camera_pos"), 1, camera.position_gl());
                    screen.draw(0, 0, glfw.width, glfw.height);
                });

            gbuffer.textures[0]->unbind();
            gbuffer.textures[1]->unbind();
            gbuffer.textures[2]->unbind();
            gbuffer.textures[3]->unbind();

            glDisable(GL_BLEND);
            light_pass_buffer.unbind();
        };
        ligthing_pass();

        glClear(GL_COLOR_BUFFER_BIT);
        screen_shader.use();
        light_pass_buffer.textures[0]->bind();
        screen.draw(0, 0, glfw.width, glfw.height);
        light_pass_buffer.textures[0]->unbind();
        screen_shader.unuse();

        glfwSwapBuffers(glfw.window);
        frame_timer.finish();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    return glmain();
}