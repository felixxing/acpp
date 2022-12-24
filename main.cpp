#include "context.hpp"

#include "texture.hpp"
#include "framebuffer.hpp"

int main(int argc, char** argv)
{
    Context context(1920, 1080);
    context.glfw_call(glfwWindowHint, GLFW_CONTEXT_VERSION_MAJOR, 4);
    context.glfw_call(glfwWindowHint, GLFW_CONTEXT_VERSION_MINOR, 5);
    context.glfw_call(glfwWindowHint, GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    context.glfw_call(glfwWindowHint, GLFW_RESIZABLE, GLFW_FALSE);
    context.glfw_call(glfwWindowHint, GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    context.create_window("ogl");

    Texture2dCreateInfo create_info;
    Texture2d textt(create_info, "res/textures/default_texture.png");

    ScreenRect sr;

    while (!context.glfw_window_call(glfwWindowShouldClose))
    {
        context.glfw_call(glfwPollEvents);

        textt.bind();
        sr.draw(0, 0, 1920, 1080);
        textt.unbind();

        context.glfw_window_call(glfwSwapBuffers);
    }

    return EXIT_SUCCESS;
}