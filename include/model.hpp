#ifndef MODEL_HPP
#define MODEL_HPP

#include "mesh.hpp"
#include "shader.hpp"

template <uint32_t MAX_INSTANCE>
class Model
{
  private:
    std::vector<Mesh<MAX_INSTANCE>*> meshes;
    std::vector<Material*> materials;

  public:
    std::string path;

    uint32_t ins_count;
    const uint32_t max_ins = MAX_INSTANCE;
    glm::mat4 ins_matrix[MAX_INSTANCE];

    Model(std::string file_path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_GenNormals);

        if (scene == nullptr)
        {
            printf("Do not load model \"%s\" from file [%p]\n", file_path.c_str(), this);
            return;
        }

        meshes.resize(scene->mNumMeshes);
        for (int i = 0; i < meshes.size(); i++)
        {
            meshes[i] = new Mesh<MAX_INSTANCE>(scene->mMeshes[i]);
            meshes[i]->load();
        }

        aiString rel_path;
        std::string prev_path = std::string(file_path.begin(), file_path.begin() + file_path.rfind('/') + 1);
        std::string final_path;
        materials.resize(scene->mNumMaterials);
        for (int i = 0; i < materials.size(); i++)
        {
            materials[i] = new Material;

            Texture2DCreateInfo material_info;

            material_info.internal_format = GL_SRGB8_ALPHA8;
            scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &rel_path);
            materials[i]->diff_map.create(material_info, prev_path + rel_path.C_Str());

            material_info.internal_format = GL_SRGB8_ALPHA8;
            scene->mMaterials[i]->GetTexture(aiTextureType_SPECULAR, 0, &rel_path);
            materials[i]->spec_map.create(material_info, prev_path + rel_path.C_Str());

            material_info.internal_format = GL_SRGB8_ALPHA8;
            scene->mMaterials[i]->GetTexture(aiTextureType_AMBIENT, 0, &rel_path);
            materials[i]->ambi_map.create(material_info, prev_path + rel_path.C_Str());

            material_info.internal_format = GL_SRGB8_ALPHA8;
            scene->mMaterials[i]->GetTexture(aiTextureType_EMISSIVE, 0, &rel_path);
            materials[i]->emis_map.create(material_info, prev_path + rel_path.C_Str());

            material_info.internal_format = GL_SRGB8_ALPHA8;
            scene->mMaterials[i]->GetTexture(aiTextureType_OPACITY, 0, &rel_path);
            materials[i]->opac_map.create(material_info, prev_path + rel_path.C_Str());
        }
    }

    ~Model()
    {
        for (int i = 0; i < meshes.size(); i++)
        {
            delete meshes[i];
        }
        for (int i = 0; i < materials.size(); i++)
        {
            delete materials[i];
        }
    }

    void draw(Shader& target_shader, bool use_material = true)
    {
        target_shader.use();

        if (use_material)
        {
            glUniform1i(target_shader.uniform("diff_map"), 0);
            glUniform1i(target_shader.uniform("spec_map"), 1);
            glUniform1i(target_shader.uniform("ambi_map"), 2);
            glUniform1i(target_shader.uniform("emis_map"), 3);
            glUniform1i(target_shader.uniform("opac_map"), 4);
        }

        for (int i = 0; i < meshes.size(); i++)
        {
            meshes[i]->draw(ins_matrix, ins_count, materials, use_material);
        }

        target_shader.unuse();
    }

    void draw()
    {
        for (int i = 0; i < meshes.size(); i++)
        {
            meshes[i]->draw(ins_matrix, ins_count, materials, false);
        }
    }
};

#endif // MODEL_HPP
