#ifndef FRAME_BUFF_HPP
#define FRAME_BUFF_HPP

#include "gls.hpp"
#include "texture.hpp"

class FrameBuffer
{
  private:
    GLID id_;
    int w_;
    int h_;

    std::vector<GLID> rbos_;
    std::vector<Texture2D> textures_;

  public:
};

#endif // FRAME_BUFF_HPP
