#ifndef MESH_HPP
#define MESH_HPP

#include "gls.hpp"
#include "ass.hpp"
#include "texture.hpp"

struct Material
{
    Texture2D diff_map;
    Texture2D spec_map;
    Texture2D ambi_map;
    Texture2D emis_map;
    Texture2D opac_map;
};

template <uint32_t MAX_INSTANCE>
class Mesh
{
  private:
    GLID VBO_POSITION;
    GLID VBO_NORMAL;
    GLID VBO_UVS;
    GLID VBO_COLORS;
    GLID VBO_INSTANCE;

    GLID VAO;
    GLID EBO;

    uint32_t vertex_count;
    std::unique_ptr<glm::vec3[]> position;
    std::unique_ptr<glm::vec3[]> normals;
    std::unique_ptr<glm::vec3[]> uvs;
    std::unique_ptr<glm::vec4[]> colors;

    uint32_t index_count;
    std::unique_ptr<uint32_t[]> indices;

    uint32_t material_index;

  public:
    Mesh(aiMesh* mesh_res)
    {
        vertex_count = mesh_res->mNumVertices;
        position = std::make_unique<glm::vec3[]>(vertex_count);
        normals = std::make_unique<glm::vec3[]>(vertex_count);
        uvs = std::make_unique<glm::vec3[]>(vertex_count);
        colors = std::make_unique<glm::vec4[]>(vertex_count);

        index_count = 3 * mesh_res->mNumFaces;
        indices = std::make_unique<uint32_t[]>(index_count);

        memcpy(position.get(), mesh_res->mVertices, sizeof(glm::vec3) * vertex_count);
        memcpy(normals.get(), mesh_res->mNormals, sizeof(glm::vec3) * vertex_count);

        if (mesh_res->mTextureCoords[0] != nullptr)
        {
            memcpy(uvs.get(), mesh_res->mTextureCoords[0], vertex_count * sizeof(glm::vec3));
        }
        else
        {
            memset(uvs.get(), 0x0, vertex_count * sizeof(glm::vec3));
        }

        if (mesh_res->mColors[0] != nullptr)
        {
            memcpy(colors.get(), mesh_res->mColors[0], vertex_count * sizeof(glm::vec4));
        }
        else
        {
            for (uint32_t i = 0; i < vertex_count; i++)
            {
                colors[i] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            }
        }

        for (uint32_t i = 0; i < mesh_res->mNumFaces; i++)
        {
            indices[3 * i] = mesh_res->mFaces[i].mIndices[0];
            indices[3 * i + 1] = mesh_res->mFaces[i].mIndices[1];
            indices[3 * i + 2] = mesh_res->mFaces[i].mIndices[2];
        }

        material_index = mesh_res->mMaterialIndex;
    }

    ~Mesh()
    {
        glDeleteVertexArrays(1, &VAO);

        glDeleteBuffers(1, &VBO_POSITION);
        glDeleteBuffers(1, &VBO_NORMAL);
        glDeleteBuffers(1, &VBO_UVS);
        glDeleteBuffers(1, &VBO_COLORS);
        glDeleteBuffers(1, &VBO_INSTANCE);
        glDeleteBuffers(1, &EBO);
    }

    void load()
    {
        glCreateBuffers(1, &VBO_POSITION);
        glNamedBufferStorage(VBO_POSITION, vertex_count * sizeof(glm::vec3), position.get(), 0);

        glCreateBuffers(1, &VBO_NORMAL);
        glNamedBufferStorage(VBO_NORMAL, vertex_count * sizeof(glm::vec3), normals.get(), 0);

        glCreateBuffers(1, &VBO_UVS);
        glNamedBufferStorage(VBO_UVS, vertex_count * sizeof(glm::vec3), uvs.get(), 0);

        glCreateBuffers(1, &VBO_COLORS);
        glNamedBufferStorage(VBO_COLORS, vertex_count * sizeof(glm::vec4), colors.get(), 0);

        glCreateBuffers(1, &VBO_INSTANCE);
        glNamedBufferStorage(VBO_INSTANCE, MAX_INSTANCE * sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);

        glCreateBuffers(1, &EBO);
        glNamedBufferStorage(EBO, index_count * sizeof(uint32_t), indices.get(), 0);

        glCreateVertexArrays(1, &VAO);
        glVertexArrayVertexBuffer(VAO, 0, VBO_POSITION, 0, sizeof(position[0]));
        glVertexArrayVertexBuffer(VAO, 1, VBO_NORMAL, 0, sizeof(normals[0]));
        glVertexArrayVertexBuffer(VAO, 2, VBO_UVS, 0, sizeof(uvs[0]));
        glVertexArrayVertexBuffer(VAO, 3, VBO_COLORS, 0, sizeof(colors[0]));
        glVertexArrayVertexBuffer(VAO, 4, VBO_INSTANCE, 0, sizeof(glm::mat4));
        glVertexArrayElementBuffer(VAO, EBO);

        for (uint32_t i = 0; i < 8; i++)
        {
            glEnableVertexArrayAttrib(VAO, i);
            glVertexArrayAttribBinding(VAO, i, i);
        }

        glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(VAO, 2, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(VAO, 3, 4, GL_FLOAT, GL_FALSE, 0);

        for (int i = 0; i < 4; i++)
        {
            glVertexArrayAttribBinding(VAO, 4 + i, 4);
            glVertexArrayBindingDivisor(VAO, 4 + i, 1);
            glVertexArrayAttribFormat(VAO, 4 + i, 4, GL_FLOAT, GL_FALSE, i * sizeof(glm::vec4));
        }
    }

    void draw(glm::mat4* ins_matrix, uint32_t count, std::vector<Material*>& materials, bool use_material = true)
    {
        glNamedBufferSubData(VBO_INSTANCE, 0, MAX_INSTANCE * sizeof(glm::mat4), ins_matrix);

        if (use_material)
        {
            materials[material_index]->diff_map.bind(0);
            materials[material_index]->spec_map.bind(1);
            materials[material_index]->ambi_map.bind(2);
            materials[material_index]->emis_map.bind(3);
            materials[material_index]->opac_map.bind(4);
        }

        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0, count);
        glBindVertexArray(0);

        if (use_material)
        {
            materials[material_index]->diff_map.unbind();
            materials[material_index]->spec_map.unbind();
            materials[material_index]->ambi_map.unbind();
            materials[material_index]->emis_map.unbind();
            materials[material_index]->opac_map.unbind();
        }
    }
};

#endif // MESH_HPP
