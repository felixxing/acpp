#ifndef BUFFERS_HPP
#define BUFFERS_HPP

#include "gls.hpp"

class UniformBuff
{
  private:
    GLID id;
    uint32_t index;
    size_t size;

    std::vector<size_t> size_order;
    std::vector<const void*> ptr_order;

  public:
    UniformBuff(uint32_t indexa, size_t sizea)
        : index(indexa),
          size(sizea)
    {
        glCreateBuffers(1, &id);
        glNamedBufferData(id, size, nullptr, GL_DYNAMIC_DRAW);
    }

    ~UniformBuff()
    {
        glDeleteBuffers(1, &id);
    }

    void load()
    {
        size_t offset = 0;
        for (uint32_t i = 0; i < size_order.size(); i++)
        {
            glNamedBufferSubData(id, offset, size_order[i], ptr_order[i]);
            offset += size_order[i];
        }

        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBindBufferRange(GL_UNIFORM_BUFFER, index, id, 0, size);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void reset()
    {
        size_order.resize(0);
        ptr_order.resize(0);
    }

    template <typename T, typename P>
    void add_data(P* data)
    {
        size_order.push_back(sizeof(T));
        ptr_order.push_back(data);
    }
};

class ShaderStorage
{
  private:
    GLID id;
    uint32_t index;
    size_t size;
    const void* data;

  public:
    ShaderStorage(size_t sizea, uint32_t indexa, const void* dataa)
        : size(sizea),
          index(indexa),
          data(dataa)
    {
        glCreateBuffers(1, &id);
        glNamedBufferData(id, size, nullptr, GL_DYNAMIC_DRAW);
    }

    ~ShaderStorage()
    {
        glDeleteBuffers(1, &id);
    }

    void load()
    {
        glNamedBufferSubData(id, 0, size, data);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, id, 0, size);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
};

#endif // BUFFERS_HPP
