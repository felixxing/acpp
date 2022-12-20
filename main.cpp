#include "context.hpp"

int main(int argc, char** argv)
{
    Context context(1920, 1080);
    context.glfw_call(glfwWindowHint, GLFW_CONTEXT_VERSION_MAJOR, 4);
    context.glfw_call(glfwWindowHint, GLFW_CONTEXT_VERSION_MINOR, 5);
    context.glfw_call(glfwWindowHint, GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    context.glfw_call(glfwWindowHint, GLFW_RESIZABLE, GLFW_FALSE);
    context.glfw_call(glfwWindowHint, GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    context.create_window("ogl");

    while (!context.glfw_windows_call(glfwWindowShouldClose))
    {
        context.glfw_call(glfwPollEvents);

        context.glfw_windows_call(glfwSwapBuffers);
    }

    return EXIT_SUCCESS;
}