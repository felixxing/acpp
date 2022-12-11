#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <unordered_map>
#include "gls.hpp"

class Texture
{
  private:
    GLID id = 0;
    GLID slot_in = 0;
    bool binded = false;

    std::string path;
    int w, h;
    int channel = -1;

  public:
    static Texture* default_texture;

    static GLenum tex_format;
    static GLenum internal_format;
    static GLenum filter;
    static GLenum wrap;

    static void rest_attrib()
    {
        Texture::tex_format = GL_RGBA;
        Texture::internal_format = GL_RGBA16F;
        Texture::filter = GL_LINEAR_MIPMAP_LINEAR;
        Texture::wrap = GL_REPEAT;
    }

    Texture()
    {
    }

    ~Texture()
    {
        if (w + h > 0 && id != 0)
        {
            glDeleteTextures(1, &id);
        }
    }

    void load(std::string file_path, uint32_t levels = 10)
    {
        static std::unordered_map<std::string, Texture*> repeat_map;
        stbi_set_flip_vertically_on_load(true);

        path = file_path;
        auto repeat_tex = repeat_map.find(file_path);
        if (repeat_tex == repeat_map.end())
        {
            unsigned char* data = stbi_load(file_path.c_str(), &w, &h, &channel, STBI_rgb_alpha);
            if (data == nullptr)
            {
                if (default_texture != nullptr)
                {
                    link(default_texture);
                }
                return;
            }

            glCreateTextures(GL_TEXTURE_2D, 1, &id);

            glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, filter);
            glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrap);
            glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrap);
            static float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

            glTextureStorage2D(id, levels, internal_format, w, h);
            glTextureSubImage2D(id, 0, 0, 0, w, h, tex_format, GL_UNSIGNED_BYTE, data);

            if (levels > 1)
            {
                glGenerateTextureMipmap(id);
            }

            stbi_image_free(data);
            data = nullptr;
            repeat_map.insert(std::pair<std::string, Texture*>(file_path, this));
        }
        else
        {
            link(repeat_tex->second);
        }
    }

    void load(int wa, int ha)
    {
        w = wa;
        h = ha;
        path = "Not an Image Texture";
        glCreateTextures(GL_TEXTURE_2D, 1, &id);

        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, filter);
        glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, filter);
        glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrap);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrap);
        static float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glTextureStorage2D(id, 1, internal_format, w, h);
        return;
    }

    void link(Texture* target)
    {
        id = target->id;
        path = target->path;
        w = -1;
        h = -1;
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

inline Texture* Texture::default_texture = nullptr;

inline GLenum Texture::tex_format = GL_RGBA;
inline GLenum Texture::internal_format = GL_RGBA16F;
inline GLenum Texture::filter = GL_LINEAR_MIPMAP_LINEAR;
inline GLenum Texture::wrap = GL_REPEAT;

#endif // TEXTURE_HPP
