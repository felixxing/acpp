#include "framebuffer.hpp"

Framebuffer::Framebuffer(int w, int h)
    : w_(w),
      h_(h)
{
    glCreateFramebuffers(1, &id_);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &id_);
}

void Framebuffer::attach_texture(const Texture2dCreateInfo& create_info, GLenum attachment)
{
    textures_.push_back(std::make_unique<Texture2d>(create_info, w_, h_));
    textures_.back()->create(create_info);
    glNamedFramebufferTexture(id_, attachment, textures_.back()->id(), 0);
}

void Framebuffer::attach_rbo(GLenum attachment, GLenum format)
{
    rbos_.push_back(0);
    glCreateRenderbuffers(1, &rbos_.back());
    glNamedRenderbufferStorage(rbos_.back(), format, w_, h_);
    glNamedFramebufferRenderbuffer(id_, attachment, GL_RENDERBUFFER, rbos_.back());
}

void Framebuffer::draw_buffers(const std::vector<GLenum>& bufs)
{
    glNamedFramebufferDrawBuffers(id_, bufs.size(), bufs.data());
}

GLID Framebuffer::id()
{
    return id_;
}

bool Framebuffer::validate()
{
    return (glCheckNamedFramebufferStatus(id_, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void Framebuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ScreenRect::ScreenRect()
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

ScreenRect::~ScreenRect() { glDeleteVertexArrays(1, &VAO); }

void ScreenRect::draw(int x, int y, int w, int h)
{
    glBindVertexArray(VAO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glViewport(x, y, w, h);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}
