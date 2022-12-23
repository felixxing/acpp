#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "gls.hpp"

class Context
{
  private:
    GLFWwindow* window_;
    int w_, h_;

  public:
    Context(int w, int h);
    ~Context();

    void create_window(const std::string& name);

    template <typename Func, typename... Args, //
              typename Return = std::invoke_result_t<Func, decltype(window_), Args...>>
    inline Return glfw_window_call(Func func, Args... args)
    {
        return func(window_, args...);
    }

    template <typename Func, typename... Args, //
              typename Return = std::invoke_result_t<Func, Args...>>
    inline Return glfw_call(Func func, Args... args)
    {
        return func(args...);
    }

    static inline void message_callback(GLenum source, GLenum type,            //
                                        GLuint id, GLenum severity,            //
                                        GLsizei length, GLchar const* message, //
                                        void const* user_param);
};

inline void Context::message_callback(GLenum source, GLenum type,            //
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

#endif // CONTEXT_HPP
