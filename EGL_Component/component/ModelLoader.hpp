#pragma once

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <SOIL2/SOIL2.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <stdexcept>
#include <iostream>

#include "macros.h"
#include "acwind.h"

namespace ModelLoader {

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    GLuint      id  = 0;
    std::string type;
    std::string path;
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& v,
         const std::vector<GLuint>& i,
         const std::vector<Texture>& t,
         const std::string& name)
        : vertices(v), indices(i), textures(t), name(name)
        { 
            setupMesh(); 
        }

    void Draw(GLuint program) const
    {
        GLuint specularNr = 1;
        GLuint diffuseNr = 1;
        for (std::size_t i = 0; i < textures.size(); ++i) {
            GLES_CHECK_ERROR(glActiveTexture(GL_TEXTURE0 + static_cast<GLuint>(i)));

            std::string number;
            const std::string& name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);
            else if (name == "texture_headtail")
                number = std::to_string(0);
            else if (name == "texture_headtailmask")
                number = std::to_string(0);
            else if (name == "texture_wavenoise")
                number = std::to_string(0);

            // 关键修复：加上 "material."
            std::string uniformName = "material." + name + number;
            // std::cout << "" << uniformName << std::endl;
            if (name == "texture_wavenoise") {
                uniformName = name + number;
            } else if (name == "texture_headtail") {
                uniformName = name + number;
            } else if (name == "texture_headtailmask") {
                uniformName = name + number;
            }
            GLES_CHECK_ERROR(glUniform1i(glGetUniformLocation(program, uniformName.c_str()),
                        static_cast<GLint>(i)));

            GLES_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, textures[i].id));
        }

        GLES_CHECK_ERROR(glBindVertexArray(VAO));
        if (instanceCount >= 1) {
            // Instanced rendering
            GLES_CHECK_ERROR(glDrawElementsInstanced(GL_TRIANGLES,
                           static_cast<GLsizei>(indices.size()),
                           GL_UNSIGNED_INT, nullptr, instanceCount));
        } else {
            // Non-instanced rendering (fallback)
            GLES_CHECK_ERROR(glDrawElements(GL_TRIANGLES,
                           static_cast<GLsizei>(indices.size()),
                           GL_UNSIGNED_INT, nullptr));
        }
        GLES_CHECK_ERROR(glBindVertexArray(0));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void setupInstanceBuffer(const std::vector<glm::mat4>& transforms) {
        if (instanceCount != 0)
            return;
        if (!instanceVBO) {
            GLES_CHECK_ERROR(glGenBuffers(1, &instanceVBO));
        }

        GLES_CHECK_ERROR(glBindVertexArray(VAO));

        GLES_CHECK_ERROR(glBindBuffer(GL_ARRAY_BUFFER, instanceVBO));
        GLES_CHECK_ERROR(glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), transforms.data(), GL_STATIC_DRAW));


        // Set up instance attributes (mat4 takes 4 vec4 slots)
        for (GLuint i = 0; i < transforms.size(); ++i) {
            GLES_CHECK_ERROR(glEnableVertexAttribArray(3 + i));
            GLES_CHECK_ERROR(glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE,
                                                  sizeof(glm::mat4),
                                                  (void*)(sizeof(glm::vec4) * i)));
            GLES_CHECK_ERROR(glVertexAttribDivisor(3 + i, 1));
        }

        GLES_CHECK_ERROR(glBindVertexArray(0));
        GLES_CHECK_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

        instanceCount = transforms.size();
    }

    // Getter for index count
    GLsizei getIndexCount() const { return static_cast<GLsizei>(indices.size()); }

    // Getter for VAO
    GLuint getVAO() const { return VAO; }

    std::vector<Vertex>  vertices;
    std::string name;

private:
    // 数据
    std::vector<GLuint>  indices;
    std::vector<Texture> textures;

    GLuint VAO = 0, VBO = 0, EBO = 0, instanceVBO = 0;
    int instanceCount = 0;

    void setupMesh()
    {
        GLES_CHECK_ERROR(glGenVertexArrays(1, &VAO));
        GLES_CHECK_ERROR(glGenBuffers      (1, &VBO));
        GLES_CHECK_ERROR(glGenBuffers      (1, &EBO));

        GLES_CHECK_ERROR(glBindVertexArray(VAO));

        GLES_CHECK_ERROR(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GLES_CHECK_ERROR(glBufferData(GL_ARRAY_BUFFER,
                     vertices.size() * sizeof(Vertex),
                     vertices.data(), GL_STATIC_DRAW));

        GLES_CHECK_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
        GLES_CHECK_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(GLuint),
                     indices.data(), GL_STATIC_DRAW));

        // layout(location = 0) Position
        GLES_CHECK_ERROR(glEnableVertexAttribArray(0));
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, Position)));
        // layout(location = 1) Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, Normal)));
        // layout(location = 2) TexCoords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, TexCoords)));

        glBindVertexArray(0);
    }
};

class Model {
public:
    explicit Model(const std::string& path, const std::vector<glm::mat4>& transforms = {}) { 
        LOGI("load model path: %s", path.c_str());
        loadModel(path, transforms);
        getAABB();
    }

    void Draw(GLuint program, const std::vector<glm::mat4>& transforms = {}) {
        for (auto& mesh : meshes) {
            mesh.Draw(program);
        }
    }

    void bindVAO() {
        for (auto& mesh : meshes) {
            GLES_CHECK_ERROR(glBindVertexArray(mesh.getVAO()));
        }
    }

    void unbindVAO() {
        for (auto& mesh : meshes) {
            GLES_CHECK_ERROR(glBindVertexArray(0));
        }
    }

    int getIndexCount() const {
        int count = 0;
        for (const auto& mesh : meshes) {
            count += mesh.getIndexCount();
        }
        return count;
    }

    std::pair<glm::vec3,glm::vec3> getAABB() 
    {
        glm::vec3 mn( 1e9), mx(-1e9);
        for (const auto& m: meshes)
            for (const auto& v: m.vertices) {
                mn = glm::min(mn,v.Position);
                mx = glm::max(mx,v.Position);
            }
        mBoundsMin = mn;
        mBoundsMax = mx;
        return {mn, mx};
    }

    glm::vec3 boundsMin() {
        return mBoundsMin;
    }

    glm::vec3 boundsMax() {
        return mBoundsMax;
    }

private:
    std::vector<Mesh>                    meshes;
    std::string                          directory;
    std::unordered_map<std::string,Texture> loadedTextures;

    glm::vec3 mBoundsMin, mBoundsMax; 

    /*** -------------------------------------------------- ***/
    /*                 Assimp 相关辅助函数                    */
    /*** -------------------------------------------------- ***/
    void loadModel(const std::string& path, const std::vector<glm::mat4>& transforms = {})
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_PreTransformVertices);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            throw std::runtime_error("Assimp error: " + std::string(importer.GetErrorString()));

        directory = std::filesystem::path(path).parent_path().string();
        processNode(scene->mRootNode, scene);
        
        // 多实例模型初始化
        for (auto& mesh : meshes) {
            if (!transforms.empty()) {
                mesh.setupInstanceBuffer(transforms);
            }
        }
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.emplace_back(processMesh(mesh, scene));
        }
        for (unsigned int i = 0; i < node->mNumChildren; ++i)
            processNode(node->mChildren[i], scene);
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex>  vertices;
        std::vector<GLuint>  indices;
        std::vector<Texture> textures;

        /* ---- 顶点 & 法线 & 纹理坐标 ---- */
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            Vertex v;
            v.Position = { mesh->mVertices[i].x,
                           mesh->mVertices[i].y,
                           mesh->mVertices[i].z };

            if (mesh->HasNormals())
                v.Normal = { mesh->mNormals[i].x,
                             mesh->mNormals[i].y,
                             mesh->mNormals[i].z };
            else
                v.Normal = { 0.f, 0.f, 1.f };

            if (mesh->mTextureCoords[0])
                v.TexCoords = { mesh->mTextureCoords[0][i].x,
                                mesh->mTextureCoords[0][i].y };
            else
                v.TexCoords = { 0.f, 0.f };

            vertices.push_back(v);
        }

        /* ---- 索引 ---- */
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j)
                indices.push_back(face.mIndices[j]);
        }

        std::string name;
        /* ---- 材质纹理 ---- */
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiString matName;
            scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, matName);
            std::cout << "mesh uses material: " << matName.C_Str() << '\n';
            name = std::string(matName.C_Str());
            
            std::cout << "Ka  : " << material->GetTextureCount(aiTextureType_AMBIENT)   << '\n';
            std::cout << "Kd  : " << material->GetTextureCount(aiTextureType_DIFFUSE)   << '\n';
            std::cout << "Ks  : " << material->GetTextureCount(aiTextureType_SPECULAR)  << '\n';
            std::cout << "Ke  : " << material->GetTextureCount(aiTextureType_EMISSIVE)  << '\n';
            std::cout << "Bump: " << material->GetTextureCount(aiTextureType_HEIGHT)     << '\n';
            LOGI("material %s: %d", name.c_str(), material->GetTextureCount(aiTextureType_AMBIENT));
            LOGI("material %s: %d", name.c_str(), material->GetTextureCount(aiTextureType_DIFFUSE));
            LOGI("material %s: %d", name.c_str(), material->GetTextureCount(aiTextureType_SPECULAR));
            LOGI("material %s: %d", name.c_str(), material->GetTextureCount(aiTextureType_EMISSIVE));
            LOGI("material %s: %d", name.c_str(), material->GetTextureCount(aiTextureType_HEIGHT));
            loadMaterialTextures(material, aiTextureType_DIFFUSE,
                                 "texture_diffuse", textures);
            loadMaterialTextures(material, aiTextureType_SPECULAR,
                                 "texture_specular", textures);
            loadMaterialTextures(material, aiTextureType_AMBIENT,
                                "texture_headtail", textures);
            loadMaterialTextures(material, aiTextureType_EMISSIVE,
                                "texture_headtailmask", textures);
            loadMaterialTextures(material, aiTextureType_HEIGHT,
                                "texture_wavenoise", textures);
        }

        return Mesh(vertices, indices, textures, name);
    }

    void loadMaterialTextures(aiMaterial* mat,
                              aiTextureType type,
                              const std::string& typeName,
                              std::vector<Texture>& textures)
    {
        for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
            aiString str;
            mat->GetTexture(type, i, &str);

            std::filesystem::path file(str.C_Str());
            std::string fullPath = (std::filesystem::path(directory) / file).string();
            LOGI("texture path: %s", fullPath.c_str());

            // 已经载入过？
            if (auto it = loadedTextures.find(fullPath); it != loadedTextures.end()) {
                textures.push_back(it->second);
                continue;
            }

            GLuint id = textureFromFile(fullPath);
            Texture tex{ id, typeName, fullPath };
            textures.push_back(tex);
            loadedTextures[fullPath] = tex;
        }
    }

    GLuint textureFromFile(const std::string& path)
    {
        GLuint textureID;
        glGenTextures(1, &textureID);

        int w, h, ch;
        unsigned char* data = SOIL_load_image(path.c_str(), &w, &h, &ch, SOIL_LOAD_AUTO);
        if (!data)
            throw std::runtime_error("SOIL2 failed: " + path + "\n" + SOIL_last_result());
        
        std::cout << "texture: " << path << " (" << w << 'x' << h << 'x' << ch << ")" << std::endl;

        GLenum format = ch == 1 ? GL_RED  :
                        ch == 3 ? GL_RGB :
                                  GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);           // RGB 对齐
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
                     format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        SOIL_free_image_data(data);
        glBindTexture(GL_TEXTURE_2D, 0);
        return textureID;
    }
};

} // namespace ModelLoader
