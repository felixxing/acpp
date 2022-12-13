#ifndef GLFW_HPP
#define GLFW_HPP

#include "gls.hpp"

class GLWindow
{
  public:
    int width, height;
    GLFWwindow* window = nullptr;

    GLWindow(int wa, int ha);
    ~GLWindow();

    void set_hint(int hint, int value);
    void load(std::string window_name);
};

inline GLWindow::GLWindow(int wa, int ha)
    : width(wa),
      height(ha)
{
    glfwInit();
}

inline GLWindow::~GLWindow()
{
}

inline void GLWindow::set_hint(int hint, int value)
{
    glfwWindowHint(hint, value);
}

inline void message_callback(GLenum source, GLenum type,            //
                             GLuint id, GLenum severity,            //
                             GLsizei length, GLchar const* message, //
                             void const* user_param)
{
    auto const src_str = [source]()
    {
        switch (source)
        {
            case GL_DEBUG_SOURCE_API:
                return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                return "WINDOW SYSTEM";
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
                return "SHADER COMPILER";
            case GL_DEBUG_SOURCE_THIRD_PARTY:
                return "THIRD PARTY";
            case GL_DEBUG_SOURCE_APPLICATION:
                return "APPLICATION";
            case GL_DEBUG_SOURCE_OTHER:
                return "OTHER";
            default:
                return "";
        }
    }();

    auto const type_str = [type]()
    {
        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR:
                return "ERROR";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                return "DEPRECATED_BEHAVIOR";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                return "UNDEFINED_BEHAVIOR";
            case GL_DEBUG_TYPE_PORTABILITY:
                return "PORTABILITY";
            case GL_DEBUG_TYPE_PERFORMANCE:
                return "PERFORMANCE";
            case GL_DEBUG_TYPE_MARKER:
                return "MARKER";
            case GL_DEBUG_TYPE_OTHER:
                return "OTHER";
            default:
                return "";
        }
    }();

    auto const severity_str = [severity]()
    {
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                return "NOTIFICATION";
            case GL_DEBUG_SEVERITY_LOW:
                return "LOW";
            case GL_DEBUG_SEVERITY_MEDIUM:
                return "MEDIUM";
            case GL_DEBUG_SEVERITY_HIGH:
                return "HIGH";
            default:
                return "";
        }
    }();

    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        return;
    }

    std::cout << fmt::format("[Source: {}]\n[Type: {}]\n[Serverity: {}]\n[ID: {}]\n{}\n\n", //
                             src_str, type_str, severity_str, id, message);
}

inline void GLWindow::load(std::string window_name)
{
    window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Do not init glad");
    }
    glfwSwapInterval(0);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_TEXTURE_2D);
    glDebugMessageCallback(message_callback, nullptr);
}

#endif // GLFW_HPP
