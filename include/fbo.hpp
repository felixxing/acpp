#ifndef FBO_HPP
#define FBO_HPP

#include "gls.hpp"
#include "texture.hpp"

class FrameBuff
{
  private:
    GLID id;
    GLID rbo;

  public:
    int w, h;

    // they shoudl not be use to determine the state of framebuff
    static GLenum tex_format;
    static GLenum internal_format;
    static GLenum filter;
    static GLenum wrap;
    static GLenum attachment;

    std::vector<Texture*> textures;

    static void reset_attrib()
    {
        FrameBuff::tex_format = GL_RGBA;
        FrameBuff::internal_format = GL_RGBA16F;
        FrameBuff::filter = GL_LINEAR;
        FrameBuff::wrap = GL_REPEAT;
        FrameBuff::attachment = GL_COLOR_ATTACHMENT0;
    }

    FrameBuff(int wa, int ha)
        : w(wa),
          h(ha)
    {
        glCreateFramebuffers(1, &id);
    }

    ~FrameBuff()
    {
        glDeleteFramebuffers(1, &id);
        for (int i = 0; i < textures.size(); i++)
        {
            delete textures[i];
        }
    }

    void attach_texture()
    {
        textures.push_back(new Texture);
        Texture::tex_format = tex_format;
        Texture::internal_format = internal_format;
        Texture::filter = filter;
        Texture::wrap = wrap;

        textures.back()->load(w, h);
        glNamedFramebufferTexture(id, attachment, textures.back()->get_id(), 0);

        Texture::rest_attrib();
        FrameBuff::reset_attrib();
    }

    void load_draws(std::vector<GLenum> buffers)
    {
        glNamedFramebufferDrawBuffers(id, buffers.size(), buffers.data());
    }

    void attach_rbo(GLenum attachment, GLenum format)
    {
        glCreateRenderbuffers(1, &rbo);
        glNamedRenderbufferStorage(rbo, format, w, h);
        glNamedFramebufferRenderbuffer(id, attachment, GL_RENDERBUFFER, rbo);
    }

    bool validate()
    {
        bool result = (glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        return result;
    }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

inline GLenum FrameBuff::tex_format = GL_RGBA;
inline GLenum FrameBuff::internal_format = GL_RGBA16F;
inline GLenum FrameBuff::filter = GL_LINEAR;
inline GLenum FrameBuff::wrap = GL_REPEAT;
inline GLenum FrameBuff::attachment = GL_COLOR_ATTACHMENT0;

class ScreenRect
{
  private:
    GLID VAO;
    GLID VBO_POS;
    GLID VBO_UV;

  public:
    ScreenRect()
    {
        float position[12] = {-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f};
        float uvs[12] = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

        // gen buffers
        glCreateBuffers(1, &VBO_POS);
        glNamedBufferData(VBO_POS, sizeof(position), position, GL_STATIC_DRAW);

        glCreateBuffers(1, &VBO_UV);
        glNamedBufferData(VBO_UV, sizeof(uvs), uvs, GL_STATIC_DRAW);

        glCreateVertexArrays(1, &VAO);
        glVertexArrayVertexBuffer(VAO, 0, VBO_POS, 0, 2 * sizeof(float));
        glVertexArrayVertexBuffer(VAO, 1, VBO_UV, 0, 2 * sizeof(float));
        glEnableVertexArrayAttrib(VAO, 0);
        glEnableVertexArrayAttrib(VAO, 1);
        glVertexArrayAttribBinding(VAO, 0, 0);
        glVertexArrayAttribBinding(VAO, 1, 1);

        glVertexArrayAttribFormat(VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 0);
    }

    ~ScreenRect()
    {
        glDeleteVertexArrays(1, &VAO);
    }

    void draw(int x, int y, int w, int h)
    {
        glBindVertexArray(VAO);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glViewport(x, y, w, h);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
    }
};

#endif // FBO_HPP
