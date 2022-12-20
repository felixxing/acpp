#include "context.hpp"

Context::Context(int w, int h)
    : w_(w),
      h_(h)
{
    glfwInit();
}

Context::~Context()
{
    glfwTerminate();
}

void Context::create_window(const std::string& name)
{
    window_ = glfwCreateWindow(w_, h_, name.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(window_);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << fmt::format("Do not Init glad\n");
    }
    glfwSwapInterval(1);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(message_callback, nullptr);
}
