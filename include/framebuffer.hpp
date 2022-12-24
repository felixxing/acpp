#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include "gls.hpp"
#include "texture.hpp"

class ScreenRect
{
  private:
    GLID VAO;
    GLID VBO_POS;
    GLID VBO_UV;

  public:
    ScreenRect();
    ~ScreenRect();

    void draw(int x, int y, int w, int h);
};

class Framebuffer
{
  private:
    GLID id_;
    int w_;
    int h_;

    std::vector<GLID> rbos_;
    std::vector<std::unique_ptr<Texture2d>> textures_;

  public:
    Framebuffer(int w, int h);
    ~Framebuffer();

    void attach_texture(const Texture2dCreateInfo& create_info, GLenum attachment);
    void attach_rbo(GLenum attachment, GLenum format);

    void draw_buffers(const std::vector<GLenum>& bufs);

    GLID id();
    bool validate();

    void bind();
    void unbind();
};

#endif // FRAME_BUFF_HPP
