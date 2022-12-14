#include <iostream>
#include <thread>
#include <cmath>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hor_Value_Slider.H>

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

int main(int argc, char** argv)
{
    Fl_Double_Window window(400, 500);
    window.end();
    window.show();
    Fl_Hor_Value_Slider light1_x(40, 0, 200, 30);
    light1_x.labelsize(22);
    light1_x.range(-200.0f, 200.0f);
    Fl_Hor_Value_Slider light1_y(40, 40, 200, 30);
    light1_y.labelsize(22);
    light1_y.range(-200.0f, 200.0f);
    Fl_Hor_Value_Slider light1_z(40, 80, 200, 30);
    light1_z.labelsize(22);
    light1_z.range(-200.0f, 200.0f);
    window.add(light1_x);
    window.add(light1_y);

    window.add(light1_z);
    Fl_Hor_Value_Slider lightd_x(40, 200, 200, 30);
    lightd_x.labelsize(22);
    lightd_x.range(-1.0f, 1.0f);
    Fl_Hor_Value_Slider lightd_y(40, 240, 200, 30);
    lightd_y.labelsize(22);
    lightd_y.range(-1.0f, 1.0f);
    Fl_Hor_Value_Slider lightd_z(40, 280, 200, 30);
    lightd_z.labelsize(22);
    lightd_z.range(-1.0f, 1.0f);
    window.add(lightd_x);
    window.add(lightd_y);
    window.add(lightd_z);

    GLWindow glfw(1920, 1080);
    glfw.set_hint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfw.set_hint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfw.set_hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfw.set_hint(GLFW_RESIZABLE, GLFW_FALSE);
    glfw.set_hint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
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

    Model<2000> cube("res/model/cube/cube.obj");
    cube.ins_count = cube.max_ins;
    for (int i = 1; i < cube.max_ins; i++)
    {
        cube.ins_matrix[i] = glm::rotate(
            glm::translate(glm::mat4(1.0f), {get_random(-100, 100), get_random(120), get_random(-100, 100)}),
            get_random(glm::radians(360.0f)), {get_random(-1, 1), get_random(1), get_random(-1, 1)});
    }

    Model<20> cube3("res/model/cube/cube.obj");
    cube3.ins_count = 1;

    Camera camera;
    camera.x = 0;
    camera.y = 0;
    camera.w = gbuffer.w;
    camera.h = gbuffer.h;
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

    PtLight pt_light1(1024);
    PtLight pt_light2(1024);
    PtLight pt_light3(1024);

    pt_light1.far = 200;
    pt_light1.light_pos = {0, 60, 0};
    pt_light1.light_color = {1, 0, 0};

    pt_light2.light_pos = {0, 20, 0};
    pt_light2.light_color = {0, 0, 1};

    pt_light3.light_pos = {0, 40, 0};
    pt_light3.light_color = {0, 1, 0};

    DirLight dir_light1(8102, 8102);
    dir_light1.color = {1, 1, 1};

    bool e_press = false;

    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    while (!glfwWindowShouldClose(glfw.window))
    {
        Fl::flush();
        pt_light1.light_pos = {light1_x.value(), light1_y.value(), light1_z.value()};
        dir_light1.direction = {lightd_x.value(), lightd_y.value(), lightd_z.value()};
        cube3.ins_matrix[0] = glm::translate(glm::mat4(1.0f), pt_light1.light_pos);

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
        if (glfwGetMouseButton(glfw.window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            cube.ins_matrix[0] =
                glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f),
                                                      camera.position + 10.0f * camera.front - glm::vec3(0, 3, 0)),
                                       glm::radians(camera.yaw), camera.up),
                           {5, 5, 5});
        }
        if (glfwGetKey(glfw.window, GLFW_KEY_F) == GLFW_PRESS)
        {
            window.show();
        }

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
            cube3.draw(gbuffer_shader);
            gbuffer.unbind();
            glDisable(GL_DEPTH_TEST);

            dir_light1.shadow_pass(
                [&]()
                {
                    sponza_model.draw();
                    cube.draw();
                });

            pt_light1.shadow_pass(
                [&]()
                {
                    sponza_model.draw();
                    cube.draw();
                });

            pt_light2.shadow_pass(
                [&]()
                {
                    sponza_model.draw();
                    cube.draw();
                });

            pt_light3.shadow_pass(
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

            pt_light2.color_pass(
                [&]()
                {
                    glUniform3fv(PtLight::color_pass_shader->uniform("camera_pos"), 1, camera.position_gl());
                    screen.draw(0, 0, glfw.width, glfw.height);
                });

            pt_light3.color_pass(
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