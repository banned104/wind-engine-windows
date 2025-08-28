#include "TextureManager.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

// 使用项目中已有的STB图片加载库
#include "../3rdparty/SOIL2/stb_image.h"

// 日志宏定义
#ifdef ANDROID
#include <android/log.h>
#define LOG_TAG "GlobalTextureManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#else
#define LOGI(fmt, ...) printf("[INFO][GlobalTextureManager] " fmt "\n", ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[ERROR][GlobalTextureManager] " fmt "\n", ##__VA_ARGS__)
#define LOGW(fmt, ...) printf("[WARN][GlobalTextureManager] " fmt "\n", ##__VA_ARGS__)
#endif

// 线程安全的单例实现
GlobalTextureManager& GlobalTextureManager::getInstance() {
    static GlobalTextureManager instance;
    return instance;
}

GlobalTextureManager::~GlobalTextureManager() {
    cleanup();
}

bool GlobalTextureManager::initialize() {
    if (m_initialized) {
        LOGW("GlobalTextureManager already initialized");
        return true;
    }

    // 获取OpenGL最大纹理单元数
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_maxTextureUnits);
    LOGI("GlobalTextureManager initialized. Max texture units: %d", m_maxTextureUnits);

    // 设置stb_image垂直翻转（OpenGL纹理坐标系）
    stbi_set_flip_vertically_on_load(true);

    m_initialized = true;
    return true;
}

bool GlobalTextureManager::loadTexture(const std::string& filePath,
                                      const std::string& textureKey,
                                      bool generateMipmap) {
    if (!m_initialized) {
        LOGE("GlobalTextureManager not initialized. Call initialize() first.");
        return false;
    }

    if (!validateFilePath(filePath)) {
        LOGE("Invalid file path: %s", filePath.c_str());
        return false;
    }

    // 生成纹理键名
    std::string key = textureKey.empty() ? generateTextureKey(filePath) : textureKey;

    // 检查是否已加载，如果已加载则增加引用计数
    if (hasTexture(key)) {
        m_textures[key]->referenceCount++;
        LOGI("Texture already loaded, reference count increased: %s (refs: %zu)",
             key.c_str(), m_textures[key]->referenceCount);
        return true;
    }

    // 加载图片数据
    int width, height, channels;
    unsigned char* data = loadImageData(filePath, width, height, channels);
    if (!data) {
        LOGE("Failed to load image: %s", filePath.c_str());
        return false;
    }

    // 创建OpenGL纹理
    GLuint textureId = createGLTexture(data, width, height, channels, generateMipmap);

    // 释放图片数据
    freeImageData(data);

    if (textureId == 0) {
        LOGE("Failed to create OpenGL texture for: %s", filePath.c_str());
        return false;
    }

    // 存储纹理信息
    auto textureInfo = std::make_unique<TextureInfo>();
    textureInfo->textureId = textureId;
    textureInfo->width = width;
    textureInfo->height = height;
    textureInfo->channels = channels;
    textureInfo->format = getGLFormat(channels);
    textureInfo->filePath = filePath;
    textureInfo->referenceCount = 1; // 初始引用计数为1

    m_textures[key] = std::move(textureInfo);

    LOGI("Texture loaded successfully: %s (%dx%d, %d channels, refs: 1)",
         key.c_str(), width, height, channels);

    return true;
}

bool GlobalTextureManager::bindToShader(const std::string& textureKey,
                                       GLuint programId,
                                       const std::string& uniformName,
                                       GLint textureUnit) {
    if (!hasTexture(textureKey)) {
        LOGE("Texture not found: %s", textureKey.c_str());
        return false;
    }

    // 获取uniform位置
    GLint uniformLocation = glGetUniformLocation(programId, uniformName.c_str());
    if (uniformLocation == -1) {
        LOGE("Uniform not found in shader: %s", uniformName.c_str());

        // 列出所有可用 Uniform 变量
        GLint uniformCount = 0;
        glGetProgramiv( programId, GL_ACTIVE_UNIFORMS, &uniformCount );
        for ( GLint i = 0; i < uniformCount; i++ ) {
            char name[256];
            GLsizei length;
            GLint size;
            GLenum type;
            glGetActiveUniform( programId, i, sizeof( name ), &length, &size, &type, name );
            LOGW( "Uniform[%d] : %s", i, name );
        }

        return false;
    }

    // 分配纹理单元
    GLint unit = (textureUnit == -1) ? allocateTextureUnit() : textureUnit;
    if (unit >= m_maxTextureUnits) {
        LOGE("No available texture units (max: %d)", m_maxTextureUnits);
        return false;
    }

    // 创建绑定信息
    ShaderBinding binding;
    binding.programId = programId;
    binding.uniformLocation = uniformLocation;
    binding.uniformName = uniformName;
    binding.textureUnit = unit;

    // 存储绑定信息
    m_shaderBindings[textureKey].push_back(binding);

    LOGI("Texture bound to shader: %s -> %s (unit %d)",
         textureKey.c_str(), uniformName.c_str(), unit);

    return true;
}

void GlobalTextureManager::activateTextures() {
    for (const auto& texturePair : m_textures) {
        const std::string& textureKey = texturePair.first;
        const TextureInfo* textureInfo = texturePair.second.get();

        // 查找该纹理的所有绑定
        auto bindingIt = m_shaderBindings.find(textureKey);
        if (bindingIt == m_shaderBindings.end()) {
            continue;
        }

        for (const auto& binding : bindingIt->second) {
            if (!binding.isValid()) {
                continue;
            }

            // 激活纹理单元
            glActiveTexture(GL_TEXTURE0 + binding.textureUnit);
            glBindTexture(GL_TEXTURE_2D, textureInfo->textureId);

            // 设置uniform值
            glUseProgram(binding.programId);
            glUniform1i(binding.uniformLocation, binding.textureUnit);
        }
    }

    // 重置为默认纹理单元
    glActiveTexture(GL_TEXTURE0);
}

const GlobalTextureManager::TextureInfo* GlobalTextureManager::getTextureInfo(const std::string& textureKey) const {
    auto it = m_textures.find(textureKey);
    return (it != m_textures.end()) ? it->second.get() : nullptr;
}

bool GlobalTextureManager::addReference(const std::string& textureKey) {
    auto it = m_textures.find(textureKey);
    if (it == m_textures.end()) {
        return false;
    }

    it->second->referenceCount++;
    LOGI("Reference added to texture: %s (refs: %zu)",
         textureKey.c_str(), it->second->referenceCount);
    return true;
}

bool GlobalTextureManager::removeReference(const std::string& textureKey) {
    auto it = m_textures.find(textureKey);
    if (it == m_textures.end()) {
        return false;
    }

    if (it->second->referenceCount > 0) {
        it->second->referenceCount--;
        LOGI("Reference removed from texture: %s (refs: %zu)",
             textureKey.c_str(), it->second->referenceCount);

        // 如果引用计数为0，自动删除纹理
        if (it->second->referenceCount == 0) {
            LOGI("Auto-removing texture with zero references: %s", textureKey.c_str());
            return removeTextureInternal(textureKey);
        }
    }

    return true;
}

bool GlobalTextureManager::forceRemoveTexture(const std::string& textureKey) {
    return removeTextureInternal(textureKey);
}

bool GlobalTextureManager::removeTextureInternal(const std::string& textureKey) {
    auto it = m_textures.find(textureKey);
    if (it == m_textures.end()) {
        return false;
    }

    // 删除OpenGL纹理
    if (it->second->textureId != 0) {
        glDeleteTextures(1, &it->second->textureId);
    }

    // 移除绑定信息
    m_shaderBindings.erase(textureKey);

    // 移除纹理信息
    m_textures.erase(it);

    LOGI("Texture removed: %s", textureKey.c_str());
    return true;
}

void GlobalTextureManager::cleanup() {
    // 删除所有OpenGL纹理
    for (const auto& pair : m_textures) {
        if (pair.second->textureId != 0) {
            glDeleteTextures(1, &pair.second->textureId);
        }
    }

    m_textures.clear();
    m_shaderBindings.clear();
    m_nextTextureUnit = 0;
    m_initialized = false;

    LOGI("GlobalTextureManager cleaned up");
}

size_t GlobalTextureManager::getTextureCount() const {
    return m_textures.size();
}

bool GlobalTextureManager::hasTexture(const std::string& textureKey) const {
    // 注意：此方法现在是非线程安全的
    return m_textures.find(textureKey) != m_textures.end();
}

std::vector<std::string> GlobalTextureManager::getTextureKeys() const {
    std::vector<std::string> keys;
    keys.reserve(m_textures.size());

    for (const auto& pair : m_textures) {
        keys.push_back(pair.first);
    }

    return keys;
}

void GlobalTextureManager::setDefaultTextureParameters(GLenum wrapS, GLenum wrapT,
                                                      GLenum minFilter, GLenum magFilter) {
    m_defaultWrapS = wrapS;
    m_defaultWrapT = wrapT;
    m_defaultMinFilter = minFilter;
    m_defaultMagFilter = magFilter;
}

std::string GlobalTextureManager::getUsageStatistics() const {

    size_t totalTextures = m_textures.size();
    size_t totalReferences = 0;
    size_t totalMemoryMB = 0;

    for (const auto& pair : m_textures) {
        const TextureInfo* info = pair.second.get();
        totalReferences += info->referenceCount;
        // 估算内存使用（宽×高×通道数×字节/像素）
        totalMemoryMB += (info->width * info->height * info->channels) / (1024 * 1024);
    }

    char buffer[512];
    snprintf(buffer, sizeof(buffer),
        "GlobalTextureManager Statistics:\n"
        "- Total Textures: %zu\n"
        "- Total References: %zu\n"
        "- Estimated GPU Memory: %zu MB\n"
        "- Next Texture Unit: %d\n"
        "- Max Texture Units: %d",
        totalTextures, totalReferences, totalMemoryMB, m_nextTextureUnit, m_maxTextureUnits);

    return std::string(buffer);
}

unsigned char* GlobalTextureManager::loadImageData(const std::string& filePath,
                                                   int& width, int& height, int& channels) {
    return stbi_load(filePath.c_str(), &width, &height, &channels, 0);
}

void GlobalTextureManager::freeImageData(unsigned char* data) {
    if (data) {
        stbi_image_free(data);
    }
}

GLuint GlobalTextureManager::createGLTexture(unsigned char* data, int width, int height,
                                           int channels, bool generateMipmap) {
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_defaultWrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_defaultWrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_defaultMinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_defaultMagFilter);

    // 上传纹理数据
    GLenum format = getGLFormat(channels);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // 生成Mipmap
    if (generateMipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error creating texture: 0x%x", error);
        glDeleteTextures(1, &textureId);
        return 0;
    }

    return textureId;
}

GLenum GlobalTextureManager::getGLFormat(int channels) const {
    switch (channels) {
        case 1: return GL_RED;
        case 2: return GL_RG;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default:
            LOGW("Unsupported channel count: %d, using RGB", channels);
            return GL_RGB;
    }
}

std::string GlobalTextureManager::generateTextureKey(const std::string& filePath) const {
    // 使用文件名作为键名
    std::filesystem::path path(filePath);
    return path.filename().string();
}

GLint GlobalTextureManager::allocateTextureUnit() {
    return m_nextTextureUnit++;
}

bool GlobalTextureManager::validateFilePath(const std::string& filePath) const {
    if (filePath.empty()) {
        return false;
    }

    // 检查文件是否存在
    std::ifstream file(filePath);
    return file.good();
}
