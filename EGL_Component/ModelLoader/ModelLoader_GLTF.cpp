#include "ModelLoader_GLTF.hpp"
#include <stdexcept>
#include <iostream>
#include <SOIL2/SOIL2.h>
#include <limits> // 需要包含这个头文件

// --- ModelGLTF Class Implementation ---

// 修改构造函数，初始化边界值
ModelGLTF::ModelGLTF(const std::string& path) 
    : m_boundsMin(std::numeric_limits<float>::max()),       // 初始化为float类型的最大最小值
      m_boundsMax(std::numeric_limits<float>::lowest()) 
{
    loadModel(path);
}

// 新增：实现获取边界的函数
glm::vec3 ModelGLTF::boundsMin() const {
    return m_boundsMin;
}

glm::vec3 ModelGLTF::boundsMax() const {
    return m_boundsMax;
}


void ModelGLTF::Draw(GLuint program) const {
    for (const auto& mesh : m_meshes) {
        mesh.Draw(program);
    }
}

void ModelGLTF::loadModel(const std::string& path) {
    LOGI("Loading GLTF model from: %s", path.c_str());
    Assimp::Importer importer;
    // GLTF 常用处理标志
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Assimp Error: " + std::string(importer.GetErrorString()));
    }

    m_directory = std::filesystem::path(path).parent_path().string();
    processNode(scene->mRootNode, scene);
    LOGI("Successfully loaded GLTF model.");
}

void ModelGLTF::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }
}

MeshGLTF ModelGLTF::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<VertexGLTF> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureGLTF> textures;

    /* ---- 顶点 & 法线 & 纹理坐标 ---- */
    /* 从 mesh 中提取出顶点 法线 纹理坐标 存入自己的 Mesh 对象中 */
    // 处理顶点
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        VertexGLTF vertex;
        vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        
        // --- 关键修改：在这里更新模型的整体边界 ---
        m_boundsMin.x = std::min(m_boundsMin.x, vertex.Position.x);
        m_boundsMin.y = std::min(m_boundsMin.y, vertex.Position.y);
        m_boundsMin.z = std::min(m_boundsMin.z, vertex.Position.z);

        m_boundsMax.x = std::max(m_boundsMax.x, vertex.Position.x);
        m_boundsMax.y = std::max(m_boundsMax.y, vertex.Position.y);
        m_boundsMax.z = std::max(m_boundsMax.z, vertex.Position.z);
        // --- 边界更新结束 ---

        if (mesh->HasNormals()) {
            vertex.Normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        }
        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
            vertex.Tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
            vertex.Bitangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }

    // 处理索引
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 处理材质
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // 1. 漫反射/基础色贴图
        auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. 镜面/金属度贴图
        auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. 法线贴图
        auto normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", scene); // Assimp often loads normal maps as height maps
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. 环境光遮蔽/粗糙度贴图
        auto ambientMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ambient", scene);
        textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());
    }

    return MeshGLTF(vertices, indices, textures);
}

std::vector<TextureGLTF> ModelGLTF::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const aiScene* scene) {
    std::vector<TextureGLTF> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string path = str.C_Str();

        if (m_textures_loaded.count(path)) {
            textures.push_back(m_textures_loaded[path]);
            continue;
        }

        TextureGLTF texture;
        if (path[0] == '*') { // Embedded texture  如果纹理包含在模型文件中 模型加载之后纹理也会加载到内存中 然后以 "*"方式命名
            int texture_index = std::stoi(path.substr(1));
            texture.id = textureFromMemory(scene->mTextures[texture_index]);
        } else { // External texture file
            texture.id = textureFromFile(m_directory + "/" + path);
        }
        texture.type = typeName;
        texture.path = path;
        textures.push_back(texture);
        m_textures_loaded[path] = texture;
    }
    return textures;
}

GLuint ModelGLTF::textureFromFile(const std::string& path) {
    GLuint textureID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    if (textureID == 0) {
        throw std::runtime_error("SOIL2 failed to load texture from file: " + path + "\n" + SOIL_last_result());
    }
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

GLuint ModelGLTF::textureFromMemory(const aiTexture* texture) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    int w, h, ch;
    unsigned char* data = SOIL_load_image_from_memory(
        reinterpret_cast<unsigned char*>(texture->pcData),
        texture->mWidth, &w, &h, &ch, SOIL_LOAD_AUTO);

    if (!data) {
        throw std::runtime_error("SOIL2 failed to load texture from memory: " + std::string(SOIL_last_result()));
    }

    GLenum format = (ch == 3) ? GL_RGB : GL_RGBA;
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SOIL_free_image_data(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}


// --- MeshGLTF Class Implementation ---

MeshGLTF::MeshGLTF(std::vector<VertexGLTF> vertices, std::vector<unsigned int> indices, std::vector<TextureGLTF> textures)
    : vertices(std::move(vertices)), indices(std::move(indices)), textures(std::move(textures)) {
    setupMesh();
}

void MeshGLTF::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexGLTF), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexGLTF), (void*)offsetof(VertexGLTF, Position));
    // Vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexGLTF), (void*)offsetof(VertexGLTF, Normal));
    // Vertex Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexGLTF), (void*)offsetof(VertexGLTF, TexCoords));
    // Vertex Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexGLTF), (void*)offsetof(VertexGLTF, Tangent));
    // Vertex Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexGLTF), (void*)offsetof(VertexGLTF, Bitangent));

    glBindVertexArray(0);
}

void MeshGLTF::Draw(GLuint program) const {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int ambientNr = 1;

    for (unsigned int i = 0; i < textures.size(); ++i) {
        //! 此处的循环变量 i 表示的是纹理单元索引; 因为不止一个纹理 所以每次都要调用glActiveTexture 
        //* 如果只有一个纹理 就不需要glActiveTexture 因为默认是 GL_TEXTURE0
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse") number = std::to_string(diffuseNr++);
        else if (name == "texture_specular") number = std::to_string(specularNr++);
        else if (name == "texture_normal") number = std::to_string(normalNr++);
        else if (name == "texture_ambient") number = std::to_string(ambientNr++);
        // glUniform1i 第二个参数就是纹理单元 GL_TEXTURE0 + i [从0开始]
        glUniform1i(glGetUniformLocation(program, (name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}