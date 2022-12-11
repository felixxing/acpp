#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <unordered_map>
#include "gls.hpp"

struct Texture2DCreateInfo
{
    GLenum target_type = GL_TEXTURE_2D;
    GLsizei levels = 10;
    GLenum internal_format = GL_RGBA16F;
    GLsizei width = -1;
    GLsizei height = -1;

    GLint wrap_s = GL_REPEAT;
    GLint wrap_t = GL_REPEAT;
    GLint wrap_r = GL_REPEAT;
    GLint mag_filter = GL_LINEAR;
    GLint min_filter = GL_LINEAR_MIPMAP_LINEAR;

    float border_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
};

class Texture2D
{
  private:
    GLID id = 0;
    GLID slot_in = 0;

    std::string path;
    int w, h;
    int channel;
    bool binded = false;
    bool src = true;

  public:
    static Texture2D* default_texture;
    ~Texture2D()
    {
        if (id != 0 && src)
        { // if the texture is created than free the resources
            glDeleteTextures(1, &id);
        }
    }

    void create(Texture2DCreateInfo create_info)
    {
        w ? false : w = create_info.width;
        h ? false : h = create_info.height;
        path.length() ? false : (path = "Not an Image Texture", 0);
        glCreateTextures(create_info.target_type, 1, &id);

        glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, create_info.mag_filter);
        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, create_info.min_filter);
        glTextureParameteri(id, GL_TEXTURE_WRAP_S, create_info.wrap_s);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, create_info.wrap_r);
        glTextureParameteri(id, GL_TEXTURE_WRAP_R, create_info.wrap_r);
        glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, create_info.border_color);

        glTextureStorage2D(id, create_info.levels, create_info.internal_format, w, h);
    }

    void create(Texture2DCreateInfo create_info, std::string file_path)
    {
        static std::unordered_map<std::string, Texture2D*> repeat_textures;
        stbi_set_flip_vertically_on_load(true);

        auto repeat_tex = repeat_textures.find(file_path);
        if (repeat_tex == repeat_textures.end())
        {
            stbi_uc* pixels = stbi_load(file_path.c_str(), //
                                        &w, &h, &channel,  //
                                        STBI_rgb_alpha);
            if (pixels == nullptr)
            {
                std::cout << fmt::format("Image {} can not be loaded\n", file_path);
                link(*default_texture);
                return;
            }

            path = file_path;
            create(create_info);
            glTextureSubImage2D(id, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            if (create_info.levels > 1)
            {
                glGenerateTextureMipmap(id);
            }

            stbi_image_free(pixels);
            repeat_textures.insert(std::pair<std::string, Texture2D*>(file_path, this));
        }
        else
        {
            link(*repeat_tex->second);
        }
    }

    void link(Texture2D& target)
    {
        path = target.path;
        id = target.id;
        w = target.w;
        h = target.h;
        channel = target.channel;
        src = false;
    }

    void bind(GLID slot = 0)
    {
        if (!binded)
        {
            glBindTextureUnit(slot, id);
            slot_in = slot;
            binded = true;
        }
    }

    void unbind()
    {
        if (binded)
        {
            glBindTextureUnit(slot_in, 0);
            binded = false;
            slot_in = 0;
        }
    }

    GLID get_id() const
    {
        return id;
    }

    bool is_binded() const
    {
        return binded;
    }
};

inline Texture2D* Texture2D::default_texture = nullptr;

#endif // TEXTURE_HPP
