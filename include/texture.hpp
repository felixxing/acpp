#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "gls.hpp"

struct Texture2dCreateInfo
{
    GLenum target_type = GL_TEXTURE_2D;
    GLenum internal_format = GL_RGBA16F;
    GLsizei levels = 10;

    GLint wrap_s = GL_REPEAT;
    GLint wrap_t = GL_REPEAT;
    GLint wrap_r = GL_REPEAT;
    GLint mag_filter = GL_LINEAR;
    GLint min_filter = GL_LINEAR_MIPMAP_LINEAR;

    float border_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
};

class Texture2d
{
  private:
    GLID id_ = 0;
    GLID slot_in_ = 0;

    std::string path_;
    int w_ = 0;
    int h_ = 0;
    int channel_ = 0;
    bool binded_ = false;
    bool src_ = true;

  public:
    static Texture2d* default_texture;

    Texture2d(const Texture2dCreateInfo& create_info, const std::string& path);
    Texture2d(const Texture2dCreateInfo& create_info, int w, int h);
    ~Texture2d();

    void create(const Texture2dCreateInfo& create_info);

    void operator=(Texture2d target);

    void bind(GLID slot = 0);
    void unbind();

    GLID id() const { return id_; }
    bool binded() const { return binded_; }

    std::string path() const { return path_; }
    int w() const { return w_; }
    int h() const { return h_; }
    int channel() const { return channel_; }
};

#endif // TEXTURE_HPP
