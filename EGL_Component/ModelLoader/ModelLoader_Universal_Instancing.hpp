#pragma once

#define __MULTITHREAD_LOAD__
#define __COMPLEX_MODEL__

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "macros.h"
#include "Component_LoadingView/OpenGL_LoadingView.hpp"
#include "CommonTypes.hpp"

// 通用顶点结构，适用于大多数现代渲染需求
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

// 通用纹理结构
struct Texture {
    GLuint id = 0;
    std::string type; // 例如: "texture_diffuse", "texture_specular", "texture_normal"
    std::string path; // 存储从模型文件中读取的原始路径
};

class Mesh {
public:
    // 网格数据
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    GLuint VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw() const;

    void setupInstance( const std::vector<InstanceData>& instanceData );
    void DrawInstanced( GLuint instanceCount );

    void updateInstance( int instanceID, const std::vector<InstanceData>& instanceData ); 

private:
    GLuint VBO, EBO;
    void setupMesh();


    // Instancing 实例化
    GLuint instanceVBO;
    bool hasInstanceData;
};

class Model {
public:
    // 构造函数，从指定路径加载任何 Assimp 支持的模型
    Model(const std::string& path);
    void Draw(GLuint program) const;

    // 获取模型AABB包围盒的边界
    glm::vec3 boundsMin() const;
    glm::vec3 boundsMax() const;
    // 缩放后的包围盒
    glm::vec3 scaled_boundsMin( float scale ) const { return m_boundsMin * scale; }
    glm::vec3 scaled_boundsMax( float scale ) const { return m_boundsMax * scale; }

    void uploadToGPU();

    void setupInstances( const std::vector<InstanceData>& instanceData );
    void DrawInstanced( GLuint program, GLuint instanceCount ) const;
    void DrawInstancedWind( GLuint program, GLuint instanceCount ) const;

    void updateInstanceData( int instanceID, const std::vector<InstanceData>& instanceData );

private:
    std::vector<Mesh> m_meshes;
    std::string m_directory;
    std::unordered_map<std::string, Texture> m_textures_loaded; // 缓存已加载的纹理，避免重复


    const aiScene* scene;
    Assimp::Importer importer;
    std::unique_ptr<LoadingViewClass> mLoadingViewProgram;


    // 模型整体的AABB包围盒
    glm::vec3 m_boundsMin;
    glm::vec3 m_boundsMax;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const aiScene* scene);

    // 纹理加载辅助函数
    GLuint textureFromFile(const std::string& path);
    GLuint textureFromMemory(const aiTexture* texture);

    // instancing
    std::vector<InstanceData> m_instanceData;
    bool m_hasInstanceData = false;


};