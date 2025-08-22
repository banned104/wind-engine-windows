#include "ModelLoader_Universal.hpp"
#include <stdexcept>
#include <SOIL2/SOIL2.h>
#include <limits>
#include <algorithm>    // 替换反斜杠

#if defined(_MSC_VER) // Microsoft Visual C++
    #define PROGRAMMATIC_BREAKPOINT() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__) // GCC or Clang
    #define PROGRAMMATIC_BREAKPOINT() __builtin_trap()
#else
    #include <csignal>
    #define PROGRAMMATIC_BREAKPOINT() raise(SIGTRAP)
#endif

// --- Model Class Implementation ---

Model::Model(const std::string& path) 
    : m_boundsMin(std::numeric_limits<float>::max()),
      m_boundsMax(std::numeric_limits<float>::lowest()) 
{
    loadModel(path);
}

glm::vec3 Model::boundsMin() const {
    return m_boundsMin;
}

glm::vec3 Model::boundsMax() const {
    return m_boundsMax;
}

#ifdef __COMPLEX_MODEL__
void Model::Draw(GLuint program) const {
    for (const auto& mesh : m_meshes) {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int ambientNr = 1;

        for (unsigned int i = 0; i < mesh.textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            std::string number;
            std::string name = mesh.textures[i].type;
            // LOGI( "--- TextureName: %s", name.c_str() );

            std::string uniformName;
            if (name == "texture_diffuse") {
                number = std::to_string(diffuseNr++);
                uniformName = "material.texture_diffuse" + number; 
            } 
            else if (name == "texture_specular") {
                number = std::to_string(specularNr++);
                uniformName = "material.texture_specular" + number; 
            } 
            else if ( name == "texture_normal" ) {
                number = std::to_string( normalNr++ );
                uniformName = "material.texture_normal" + number;
            }
            else if ( name == "texture_ambient" ) {
                number = std::to_string( ambientNr++ );
                uniformName = "material.texture_ambient" + number;
            }
            // glUniform1i CPU传递1个整数(integer) i给GPU 对应的Unifrom变量位置由 glGetUniformLocation 通过字符串匹配找出
            // ! 这个整数就是 GL_TEXTURE_n  告诉着色器这个 unifomName的变量(uniform变量在Shader中只是一个指针) 它应该指向的GPU内存是哪一块(这就是glUniform1i的作用)
            // ! glTexImage2D 才是将纹理数据从 RAM 通过CPU 传递到GPU RAM 的方法
            // TODO glGetUniformLocation 是一个耗时操作 最好不要在渲染循环中调用 应该在着色器链接的时候就赋值
            glUniform1i(glGetUniformLocation(program, uniformName.c_str()), i);
            glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
        }
        
        // 现在调用 Mesh 的绘制方法 它只负责绘制几何体
        mesh.Draw();
        // 绘制一个Mesh结束之后需要清理纹理绑定 否则着色器会同样采用
        for (unsigned int i = 0; i < mesh.textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0); // 将一个空的纹理对象绑定到单元上
        }
    }
    glActiveTexture(GL_TEXTURE0);
}
#else
void Model::Draw(GLuint program) const {
    for (const auto& mesh : m_meshes) {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int ambientNr = 1;

        for (unsigned int i = 0; i < mesh.textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            std::string number;
            std::string name = mesh.textures[i].type;
            // LOGI( "--- TextureName: %s", name.c_str() );

            if (name == "texture_diffuse") number = std::to_string(diffuseNr++);
            else if (name == "texture_specular") number = std::to_string(specularNr++);
            else if (name == "texture_normal") number = std::to_string(normalNr++);
            else if (name == "texture_ambient") number = std::to_string(ambientNr++);
            
            glUniform1i(glGetUniformLocation(program, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
        }
        
        // 现在调用 Mesh 的绘制方法，它只负责绘制几何体
        mesh.Draw();
    }
    glActiveTexture(GL_TEXTURE0);
}
#endif

void Model::loadModel(const std::string& path) {
    LOGI("Loading model from: %s", path.c_str());
    
    // 使用一组通用的后处理标志，适用于大多数模型格式
    #ifdef __MULTITHREAD_LOAD__     // 如果需要多线程加载 则将opengl相关的方法放到主线程中调用
    scene = importer.ReadFile(path,
        aiProcess_Triangulate |           // 将所有图元转换为三角形
        aiProcess_GenSmoothNormals |      // 如果模型没有法线，则生成平滑法线
        aiProcess_FlipUVs |               // 翻转Y轴的纹理坐标
        aiProcess_CalcTangentSpace);      // 计算切线和副切线，用于法线贴图
    #else
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |           // 将所有图元转换为三角形
        aiProcess_GenSmoothNormals |      // 如果模型没有法线，则生成平滑法线
        aiProcess_FlipUVs |               // 翻转Y轴的纹理坐标
        aiProcess_CalcTangentSpace);      // 计算切线和副切线，用于法线贴图
    #endif

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Assimp Error: " + std::string(importer.GetErrorString()));
    }

    m_directory = std::filesystem::path(path).parent_path().string();
    LOGI("Successfully loaded model to RAM.");

    #ifndef __MULTITHREAD_LOAD__
    processNode(scene->mRootNode, scene);
    LOGI("Successfully loaded model.");
    #endif
}

#ifdef __MULTITHREAD_LOAD__
// RAM to Graphic RAM 
void Model::uploadToGPU() {
    // PROGRAMMATIC_BREAKPOINT();
    processNode(scene->mRootNode, scene);
    LOGI("Successfully loaded model to Graphics RAM.");
}

#endif


void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }

    LOGI( "Meshes quantities: %d", static_cast<int>(m_meshes.size()) );
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        
        // 更新模型的整体AABB包围盒
        m_boundsMin = glm::min(m_boundsMin, vertex.Position);
        m_boundsMax = glm::max(m_boundsMax, vertex.Position);

        if (mesh->HasNormals()) {
            vertex.Normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        }
        if (mesh->mTextureCoords[0]) { // 检查是否存在纹理坐标
            vertex.TexCoords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
            if (mesh->HasTangentsAndBitangents()) {
                vertex.Tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
                vertex.Bitangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
            }
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        
        auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        
        auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        
        auto normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", scene);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        
        auto ambientMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ambient", scene);
        textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const aiScene* scene) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string path = str.C_Str();

        std::replace(path.begin(), path.end(), '\\', '/');

        if (m_textures_loaded.count(path)) {
            textures.push_back(m_textures_loaded[path]);
            continue;
        }

        Texture texture;
        // LOGI( "Man what can i say! -> %s",path.c_str() );
        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture( str.C_Str() );

        if (  embeddedTexture != nullptr ) {
            LOGI( "Founded embedded texture : %s", path.c_str() );
            texture.id = textureFromMemory( embeddedTexture );
        } else { // 处理外部纹理文件
            LOGI( "Founded texture : %s", path.c_str() );
            texture.id = textureFromFile(m_directory + "/" + path);
        }
        texture.type = typeName;
        texture.path = path;
        textures.push_back(texture);
        m_textures_loaded[path] = texture;
    }
    return textures;
}

GLuint Model::textureFromFile(const std::string& path) {
    GLuint textureID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    if (textureID == 0) {
        LOGE("SOIL2 failed to load texture from file: %s\nError: %s", path.c_str(), SOIL_last_result());
        return 0;
    }
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

GLuint Model::textureFromMemory(const aiTexture* texture) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    int w, h, ch;
    // Assimp 通常将嵌入式纹理存储为压缩格式（如.png），mWidth是压缩后的大小，需要SOIL2从内存解压
    // LOGI( "Man what can i say! -> %s", reinterpret_cast<unsigned char*>(texture->pcData) );
    unsigned char* data = SOIL_load_image_from_memory(
        reinterpret_cast<unsigned char*>(texture->pcData),
        texture->mWidth, &w, &h, &ch, SOIL_LOAD_AUTO);

    if (!data) {
        LOGE("SOIL2 failed to load texture from memory. Error: %s", SOIL_last_result());
        return 0;
    }

    GLenum format = (ch == 4) ? GL_RGBA : GL_RGB;
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


// --- Mesh Class Implementation ---

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    : vertices(std::move(vertices)), indices(std::move(indices)), textures(std::move(textures)) {
    setupMesh();
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // 设置顶点属性指针
    // 位置
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    // 法线
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // 纹理坐标
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // 切线
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // 副切线
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);
}

// 修改 Mesh::Draw，移除所有纹理逻辑，只保留绘制命令
void Mesh::Draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}