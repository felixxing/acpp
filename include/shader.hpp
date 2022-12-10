#ifndef SHADER_HPP
#define SHADER_HPP

#include "gls.hpp"

class Shader
{
  private:
    GLID program;
    std::vector<GLID> modules;

  public:
    Shader()
    {
        program = glCreateProgram();
    }

    ~Shader()
    {
        glDeleteProgram(program);
    }

    void atatch_module(GLenum type, std::string file_path)
    {
        if (modules.size() < 3)
        {
            int status = 0;
            GLID shader_id;
            char error_msg[512];

            Load_File file(file_path);
            shader_id = glCreateShader(type);
            glShaderSource(shader_id, 1, &file.get_data(), nullptr);
            glCompileShader(shader_id);

            glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
            if (!status)
            {
                glGetShaderInfoLog(shader_id, 512, nullptr, error_msg);
                printf("Do not compile shader: %s\n --> Path: %s\n", error_msg, file_path.c_str());

                glDeleteShader(shader_id);
                return;
            }

            glAttachShader(program, shader_id);

            modules.push_back(shader_id);
        }
    }

    void link()
    {
        int status = 0;
        char error_msg[512];

        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (!status)
        {
            glGetProgramInfoLog(program, 512, nullptr, error_msg);
            printf("Do not link shader program: %s\n", error_msg);
        }

        for (int i = 0; i < modules.size(); i++)
        {
            glDeleteShader(modules[i]);
        }
    }

    void use()
    {
        glUseProgram(program);
    }

    GLID uniform(std::string name)
    {
        GLID result = glGetUniformLocation(program, name.c_str());
        return result;
    }

    static void unuse()
    {
        glUseProgram(0);
    }
};

#endif // SHADER_HPP
