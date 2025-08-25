#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include<glad/glad.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// #include "../macros.h" // 假设你的 LOGI/LOGE 在这里
#include "macros.h"

// 专门为 GLTF 设计的顶点结构，包含切线和副切线，用于法线贴图
struct VertexGLTF {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct TextureGLTF {
    GLuint id = 0;
    std::string type; // e.g., "texture_diffuse", "texture_normal"
    std::string path; // 存储 assimp 提供的原始路径
};

class MeshGLTF {
public:
    // 网格数据
    std::vector<VertexGLTF> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureGLTF> textures;
    GLuint VAO;

    MeshGLTF(std::vector<VertexGLTF> vertices, std::vector<unsigned int> indices, std::vector<TextureGLTF> textures);
    void Draw(GLuint program) const;

private:
    GLuint VBO, EBO;
    void setupMesh();
};

class ModelGLTF {
public:
    // 构造函数，直接从路径加载模型
    ModelGLTF(const std::string& path);
    void Draw(GLuint program) const;

    // 新增：获取模型边界的公有函数
    glm::vec3 boundsMin() const;
    glm::vec3 boundsMax() const;

private:
    std::vector<MeshGLTF> m_meshes;
    std::string m_directory;
    std::unordered_map<std::string, TextureGLTF> m_textures_loaded; // 确保纹理只加载一次

    // 新增：存储模型整体边界的私有变量
    glm::vec3 m_boundsMin;
    glm::vec3 m_boundsMax;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    MeshGLTF processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<TextureGLTF> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const aiScene* scene);

    // 纹理加载辅助函数
    GLuint textureFromFile(const std::string& path);
    GLuint textureFromMemory(const aiTexture* texture);
};