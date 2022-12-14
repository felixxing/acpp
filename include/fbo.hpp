#ifndef FBO_HPP
#define FBO_HPP

#include "gls.hpp"
#include "texture.hpp"

enum GBufferSlot
{
    POSITION = 0,
    NORMAL = 1,
    COLOR = 2,
    SPECULAR = 3,
    SHADOW = 4
};

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
        glNamedBufferStorage(VBO_POS, sizeof(position), position, 0);

        glCreateBuffers(1, &VBO_UV);
        glNamedBufferStorage(VBO_UV, sizeof(uvs), uvs, 0);

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

class Framebuffer
{
  private:
    GLID id;

  public:
    int w, h;
    std::vector<Texture2D*> textures;
    std::vector<GLID> rbos;

    Framebuffer(int wa, int ha)
        : w(wa),
          h(ha)
    {
        glCreateFramebuffers(1, &id);
    }

    ~Framebuffer()
    {
        glDeleteFramebuffers(1, &id);
        for (int i = 0; i < textures.size(); i++)
        {
            delete textures[i];
        }
    }

    void attach_texture(Texture2DCreateInfo create_info, GLenum attachment)
    {
        textures.push_back(new Texture2D());
        textures.back()->create(create_info);
        glNamedFramebufferTexture(id, attachment, textures.back()->get_id(), 0);
    }

    void attach_rbo(GLenum attachment, GLenum format)
    {
        rbos.push_back(0);
        glCreateRenderbuffers(1, &rbos.back());
        glNamedRenderbufferStorage(rbos.back(), format, w, h);
        glNamedFramebufferRenderbuffer(id, attachment, GL_RENDERBUFFER, rbos.back());
    }

    void draw_buffers(std::vector<GLenum> bufs)
    {
        glNamedFramebufferDrawBuffers(id, bufs.size(), bufs.data());
    }

    bool validate()
    {
        return (glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLID get_id() const
    {
        return id;
    }
};

#endif // FBO_HPP
