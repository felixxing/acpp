#include "texture.hpp"

#include <stb/stb_image.h>

inline Texture2d* Texture2d::default_texture = nullptr;

Texture2d::Texture2d(const Texture2dCreateInfo& create_info, const std::string& path)
    : path_(path)
{
    static std::unordered_map<std::string, Texture2d*> repeat_textures;
    stbi_set_flip_vertically_on_load(true);

    auto repeat_tex = repeat_textures.find(path);
    if (repeat_tex == repeat_textures.end())
    {
        stbi_uc* pixels = stbi_load(path.c_str(),        //
                                    &w_, &h_, &channel_, //
                                    STBI_rgb_alpha);
        if (pixels == nullptr)
        {
            std::cout << fmt::format("Image {} can not be loaded\n", path);
            *this = *default_texture;
            return;
        }

        create(create_info);
        glTextureSubImage2D(id_, 0, 0, 0, w_, h_, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        if (create_info.levels > 1)
        {
            glGenerateTextureMipmap(id_);
        }

        stbi_image_free(pixels);
        repeat_textures.insert(std::pair<std::string, Texture2d*>(path, this));
    }
    else
    {
        *this = *repeat_tex->second;
    }
}

Texture2d::Texture2d(const Texture2dCreateInfo& create_info, int w, int h)
    : w_(w),
      h_(h)
{
    create(create_info);
}

Texture2d::~Texture2d()
{
    if (id_ && src_)
    {
        // if the texture is created than free the resources
        glDeleteTextures(1, &id_);
    }
}

void Texture2d::create(const Texture2dCreateInfo& create_info)
{
    path_.length() ? false : (path_ = "Not an Image Texture", 0);
    glCreateTextures(create_info.target_type, 1, &id_);

    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, create_info.mag_filter);
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, create_info.min_filter);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, create_info.wrap_s);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, create_info.wrap_r);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_R, create_info.wrap_r);
    glTextureParameterfv(id_, GL_TEXTURE_BORDER_COLOR, create_info.border_color);

    glTextureStorage2D(id_, create_info.levels, create_info.internal_format, w_, h_);
}

void Texture2d::operator=(Texture2d target)
{
    path_ = target.path();
    id_ = target.id();
    w_ = target.w();
    h_ = target.h();
    channel_ = target.channel();
    src_ = false;
}

void Texture2d::bind(GLID slot)
{
    if (!binded_)
    {
        glBindTextureUnit(slot, id_);
        slot_in_ = slot;
        binded_ = true;
    }
}

void Texture2d::unbind()
{
    if (binded_)
    {
        glBindTextureUnit(slot_in_, 0);
        binded_ = false;
        slot_in_ = 0;
    }
}
