#ifndef GLS_HPP
#define GLS_HPP

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <fmt/core.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include "glad/glad.h"

#define GLM_FORCE_INLINE
#define GLM_FORCE_XYZW_ONLY

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tools.hpp"

using GLID = GLuint;

void gl_check_error(const char* file, int line);

#ifndef NO_GL_TESTING
#ifndef NDEBUG
#define glc(x) \
    x;         \
    gl_check_error(__FILE__, __LINE__)
#else
#define glc(x) x
#endif
#else
#define glc(x)
#endif

inline void gl_check_error(const char* file, int line)
{
    GLenum errorCode;
    const char* error;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        switch (errorCode)
        {
            case GL_INVALID_ENUM:
            {
                error = "INVALID_ENUM";
                break;
            }
            case GL_INVALID_VALUE:
            {
                error = "INVALID_VALUE";
                break;
            }
            case GL_INVALID_OPERATION:
            {
                error = "INVALID_OPERATION";
                break;
            }
            case GL_STACK_OVERFLOW:
            {
                error = "STACK_OVERFLOW";
                break;
            }
            case GL_STACK_UNDERFLOW:
            {
                error = "STACK_UNDERFLOW";
                break;
            }
            case GL_OUT_OF_MEMORY:
            {
                error = "OUT_OF_MEMORY";
                break;
            }
            case GL_INVALID_FRAMEBUFFER_OPERATION:
            {
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
            }
            default:
            {
                break;
            }
        }
        printf("[%s] %s | %s (Line: %d)\n", __TIME__, error, file, line);
    }
}

#endif // GLS_HPP
