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

template <int aa>
class aav
{
    int ccc[aa];

  public:
    void out()
    {
        std::cout << sizeof(ccc[0]) << std::endl;
    }
};

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
    for (int i = 0; i < 4; i++)
    {
        fbo.attachment = GL_COLOR_ATTACHMENT0 + i;
        fbo.attach_texture();
    }
    fbo.load_attatchmens();
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

    Model<20> mmm("res/model/sponza/sponza.obj");
    mmm.ins_count = 1;
    mmm.ins_matrix[0] = glm::mat4(1.0f);
    mmm.ins_matrix[0] = glm::scale(mmm.ins_matrix[0], {0.1, 0.1, 0.1});

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
    lb.pt_lights_count = 1;

    ShaderStorage light_ssbo(sizeof(lb), 2, &lb);

    double m_x = 0;
    double m_y = 0;
    glfwGetCursorPos(glfw.window, &m_x, &m_y);
    double m_x_p = m_x;
    double m_y_p = m_y;

    Timer frame_timer;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
            fbo.bind();

            glEnable(GL_DEPTH_TEST);
            glClearColor(0.0, 0.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, fbo.w, fbo.h);

            mmm.draw(mmm_shader);

            fbo.unbind();
            glDisable(GL_DEPTH_TEST);
        }

        light_ssbo.load();
        light_pass.use();

        glUniform1i(light_pass.uniform("position"), 0);
        glUniform1i(light_pass.uniform("normal"), 1);
        glUniform1i(light_pass.uniform("colors"), 2);
        glUniform1i(light_pass.uniform("specs"), 3);
        glUniform3fv(light_pass.uniform("camera_pos"), 1, camera.position_gl());

        fbo.textures[0]->bind(0);
        fbo.textures[1]->bind(1);
        fbo.textures[2]->bind(2);
        fbo.textures[3]->bind(3);
        fbo_screen.draw(fbo.w / 4, 0, 3 * fbo.w / 4, fbo.h);
        fbo.textures[0]->unbind();
        fbo.textures[1]->unbind();
        fbo.textures[2]->unbind();
        fbo.textures[3]->unbind();
        light_pass.unuse();

        fbo_shader.use();
        for (int i = 0; i < 4; i++)
        {
            fbo.textures[i]->bind();
            fbo_screen.draw(0, i * fbo.h / 4, fbo.w / 4, fbo.h / 4);
            fbo.textures[i]->unbind();
        }
        fbo_shader.unuse();

        glfwSwapBuffers(glfw.window);
        frame_timer.finish();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}