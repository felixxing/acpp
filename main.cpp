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
        for (int i = 0; i < 4; i++)
        {
            Texture2DCreateInfo create_info;
            create_info.levels = 1;
            create_info.min_filter = GL_LINEAR;
            create_info.width = gbuffer.w;
            create_info.height = gbuffer.h;
            gbuffer.attach_texture(create_info, GL_COLOR_ATTACHMENT0 + i);
        }

        // attach depth map
        Texture2DCreateInfo gbuffer_depth_create_info;
        gbuffer_depth_create_info.internal_format = GL_DEPTH_COMPONENT32F;
        gbuffer_depth_create_info.wrap_s = GL_CLAMP_TO_BORDER;
        gbuffer_depth_create_info.wrap_r = GL_CLAMP_TO_BORDER;
        gbuffer_depth_create_info.wrap_t = GL_CLAMP_TO_BORDER;
        gbuffer_depth_create_info.min_filter = GL_LINEAR;
        gbuffer_depth_create_info.width = gbuffer.w;
        gbuffer_depth_create_info.height = gbuffer.h;
        gbuffer_depth_create_info.levels = 1;
        gbuffer.attach_texture(gbuffer_depth_create_info, GL_DEPTH_ATTACHMENT);
        gbuffer.draw_buffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3});
        gbuffer_validation = gbuffer.validate();
    };
    create_gbuffer();

    Framebuffer light_pass_buffer(glfw.width, glfw.height);
    bool light_pass_buffer_validation = false;
    auto create_lighting_pass_buffer = [&]()
    {
        Texture2DCreateInfo create_info;
        create_info.levels = 1;
        create_info.min_filter = GL_LINEAR;
        create_info.width = light_pass_buffer.w;
        create_info.height = light_pass_buffer.h;
        light_pass_buffer.attach_texture(create_info, GL_COLOR_ATTACHMENT0);
        light_pass_buffer.attach_rbo(GL_DEPTH_ATTACHMENT, GL_DEPTH24_STENCIL8);

        light_pass_buffer_validation = light_pass_buffer.validate();
    };
    create_lighting_pass_buffer();

    ScreenRect screen;

    Shader screen_shader;
    Shader gbuffer_shader;
    // Shader dir_light_pass_shader;
    Shader pt_light_pass_shader;
    auto init_shaders = [&]()
    {
        screen_shader.atatch_module(GL_VERTEX_SHADER, "res/shader/new/screen.vert");
        screen_shader.atatch_module(GL_FRAGMENT_SHADER, "res/shader/new/screen.frag");
        screen_shader.link();

        gbuffer_shader.atatch_module(GL_VERTEX_SHADER, "res/shader/new/gbuffer.vert");
        gbuffer_shader.atatch_module(GL_FRAGMENT_SHADER, "res/shader/new/gbuffer.frag");
        gbuffer_shader.link();

        // dir_light_pass_shader.atatch_module(GL_VERTEX_SHADER, "res/shader/new/dir_light_pass.vert");
        // dir_light_pass_shader.atatch_module(GL_FRAGMENT_SHADER, "res/shader/new/dir_light_pass.frag");
        // dir_light_pass_shader.link();

        pt_light_pass_shader.atatch_module(GL_VERTEX_SHADER, "res/shader/new/light.vert");
        pt_light_pass_shader.atatch_module(GL_FRAGMENT_SHADER, "res/shader/new/pt_light_pass.frag");
        pt_light_pass_shader.link();
    };
    init_shaders();

    Model<1> sponza_model("res/model/sponza/sponza.obj");
    sponza_model.ins_count = 1;
    sponza_model.ins_matrix[0] = glm::scale(glm::mat4(1.0f), {0.1f, 0.1f, 0.1f});

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

        auto draw_gbuffer = [&]()
        {
            glEnable(GL_DEPTH_TEST);
            gbuffer.bind();
            glCullFace(GL_BACK);
            glClearColor(0.0, 0.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, gbuffer.w, gbuffer.h);

            sponza_model.draw(gbuffer_shader);

            gbuffer.unbind();
            glDisable(GL_DEPTH_TEST);
        };
        draw_gbuffer();

        auto ligthing_pass = [&]()
        {
            pt_light_pass_shader.use();

            // set texture uniforms
            glUniform1i(pt_light_pass_shader.uniform("positions"), 0);
            glUniform1i(pt_light_pass_shader.uniform("normals"), 1);
            glUniform1i(pt_light_pass_shader.uniform("colors"), 2);
            glUniform1i(pt_light_pass_shader.uniform("specs"), 3);
            glUniform3fv(pt_light_pass_shader.uniform("camera_pos"), 1, camera.position_gl());

            // bind gbuffer
            gbuffer.textures[0]->bind(0);
            gbuffer.textures[1]->bind(1);
            gbuffer.textures[2]->bind(2);
            gbuffer.textures[3]->bind(3);

            screen.draw(0, 0, glfw.width, glfw.height);

            gbuffer.textures[0]->unbind();
            gbuffer.textures[1]->unbind();
            gbuffer.textures[2]->unbind();
            gbuffer.textures[3]->unbind();

            pt_light_pass_shader.unuse();
        };
        ligthing_pass();

        glfwSwapBuffers(glfw.window);
        frame_timer.finish();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    { /*
     GLWindow glfw(1920, 1080);
     glfw.set_hint(GLFW_CONTEXT_VERSION_MAJOR, 4);
     glfw.set_hint(GLFW_CONTEXT_VERSION_MINOR, 6);
     glfw.set_hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
     glfw.set_hint(GLFW_RESIZABLE, GLFW_FALSE);
     glfw.set_hint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
     glfw.load("deffer shading");

     Texture2D default_tex2d;
     Texture2DCreateInfo default_create_info;
     default_tex2d.create(default_create_info, "res/textures/white.png");
     Texture2D::default_texture = &default_tex2d;

     Framebuffer gbuffer(1920, 1080);
     for (int i = 0; i < 4; i++)
     {
         Texture2DCreateInfo create_info;
         create_info.levels = 1;
         create_info.min_filter = GL_LINEAR;
         create_info.width = gbuffer.w;
         create_info.height = gbuffer.h;
         gbuffer.attach_texture(create_info, GL_COLOR_ATTACHMENT0 + i);
     }
     Texture2DCreateInfo gbuffer_depth_create_info;
     gbuffer_depth_create_info.internal_format = GL_DEPTH_COMPONENT32F;
     gbuffer_depth_create_info.wrap_s = GL_CLAMP_TO_BORDER;
     gbuffer_depth_create_info.wrap_r = GL_CLAMP_TO_BORDER;
     gbuffer_depth_create_info.wrap_t = GL_CLAMP_TO_BORDER;
     gbuffer_depth_create_info.min_filter = GL_LINEAR;
     gbuffer_depth_create_info.width = gbuffer.w;
     gbuffer_depth_create_info.height = gbuffer.h;
     gbuffer_depth_create_info.levels = 1;
     gbuffer.attach_texture(gbuffer_depth_create_info, GL_DEPTH_ATTACHMENT);
     gbuffer.draw_buffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3});
     bool gbuffer_validation = gbuffer.validate();

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
     mmm2.ins_matrix[0] = glm::translate(mmm2.ins_matrix[0], {1, 0.5, 0.0});
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
     const int shadow_w = 4096;
     const int shadow_h = 4096;
     Framebuffer depth_buffer(shadow_w, shadow_h);
     Texture2DCreateInfo depth_create_info;
     depth_create_info.internal_format = GL_DEPTH_COMPONENT32F;
     depth_create_info.wrap_s = GL_CLAMP_TO_BORDER;
     depth_create_info.wrap_r = GL_CLAMP_TO_BORDER;
     depth_create_info.wrap_t = GL_CLAMP_TO_BORDER;
     depth_create_info.min_filter = GL_LINEAR;
     depth_create_info.width = shadow_w;
     depth_create_info.height = shadow_h;
     depth_create_info.levels = 1;
     depth_buffer.attach_texture(depth_create_info, GL_DEPTH_ATTACHMENT);
     bool depth_buffer_validation = depth_buffer.validate();

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
         float near_plane = 1.0f, far_plane = 40.0f;
         glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
         glm::mat4 lightView = glm::lookAt(-lb.dir_lights[0].direction, //
                                           glm::vec3(0.0f, 0.0f, 0.0f), //
                                           glm::vec3(0.0f, 1.0f, 0.0f));
         glm::mat4 light_space = lightProjection * lightView;

         {
             glEnable(GL_DEPTH_TEST);

             depth_buffer.bind();
             glCullFace(GL_FRONT);
             glClear(GL_DEPTH_BUFFER_BIT);
             glViewport(0, 0, shadow_w, shadow_h);
             depth_shader.use();
             glUniformMatrix4fv(depth_shader.uniform("light_space"), 1, GL_FALSE, glm::value_ptr(light_space));
             mmm2.draw(depth_shader);
             depth_buffer.unbind();

             glEnable(GL_DEPTH_TEST);
             gbuffer.bind();
             glCullFace(GL_BACK);
             glClearColor(0.0, 0.0, 0.0, 1.0);
             glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
             glViewport(0, 0, gbuffer.w, gbuffer.h);
             mmm_shader.use();
             glUniformMatrix4fv(mmm_shader.uniform("light_space"), 1, GL_FALSE, glm::value_ptr(light_space));
             mmm2.draw(mmm_shader);
             gbuffer.unbind();
             glDisable(GL_DEPTH_TEST);
         }

         light_ssbo.load();
         light_pass.use();

         glUniform1i(light_pass.uniform("positions"), 0);
         glUniform1i(light_pass.uniform("normals"), 1);
         glUniform1i(light_pass.uniform("colors"), 2);
         glUniform1i(light_pass.uniform("specs"), 3);
         glUniform1i(light_pass.uniform("shadows"), 4);

         glUniform3fv(light_pass.uniform("camera_pos"), 1, camera.position_gl());
         glUniformMatrix4fv(light_pass.uniform("light_space"), 1, GL_FALSE, glm::value_ptr(light_space));

         for (int i = 0; i < 4; i++)
         {
             gbuffer.textures[i]->bind(i);
         }
         depth_buffer.textures[0]->bind(4);
         fbo_screen.draw(gbuffer.w / 4, 0, 3 * gbuffer.w / 4, 3 * gbuffer.h / 4);
         // fbo_screen.draw(0, 0, fbo.w, fbo.h);
         for (int i = 0; i < 4; i++)
         {
             gbuffer.textures[i]->unbind();
         }
         depth_buffer.textures[0]->unbind();
         light_pass.unuse();

         fbo_shader.use();
         for (int i = 0; i < 4; i++)
         {
             gbuffer.textures[i]->bind();
             fbo_screen.draw(0, i * gbuffer.h / 4, gbuffer.w / 4, gbuffer.h / 4);
             gbuffer.textures[i]->unbind();
         }
         fbo_shader.unuse();

         glfwSwapBuffers(glfw.window);
         frame_timer.finish();
     }

     glfwTerminate();
     */
    }

    return glmain();
}