#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <GLES3/gl3.h>

/**
 * @brief 全局纹理管理器 - 单例模式
 *
 * 负责管理全局独立纹理（非3D模型绑定的纹理），提供纹理加载、
 * 管理和Shader绑定功能。与ModelLoader中的纹理管理分离，专注于独立纹理资源。
 *
 * 设计特点：
 * - 单例模式：全局唯一实例，统一管理所有独立纹理
 * - 资源去重：自动检测并避免重复加载相同纹理
 * - 延迟初始化：在首次使用时才创建实例
 * - 自动清理：程序结束时自动释放所有资源
 *
 * 使用场景：
 * - UI纹理、背景纹理、粒子纹理等独立纹理
 * - 不与特定3D模型绑定的通用纹理资源
 * - 需要在多个渲染组件间共享的纹理
 */
class GlobalTextureManager {
public:
    /**
     * @brief 纹理信息结构体
     */
    struct TextureInfo {
        GLuint textureId = 0;           // OpenGL纹理ID
        int width = 0;                  // 纹理宽度
        int height = 0;                 // 纹理高度
        int channels = 0;               // 通道数
        GLenum format = GL_RGB;         // 纹理格式
        std::string filePath;           // 原始文件路径
        size_t referenceCount = 0;      // 引用计数

        bool isValid() const { return textureId != 0; }
    };

    /**
     * @brief Shader绑定信息结构体
     */
    struct ShaderBinding {
        GLuint programId = 0;           // Shader程序ID
        GLint uniformLocation = -1;    // uniform变量位置
        std::string uniformName;       // uniform变量名
        GLint textureUnit = 0;         // 纹理单元

        bool isValid() const { return uniformLocation != -1; }
    };

public:
    /**
     * @brief 获取全局单例实例
     *
     * 单例获取方法，使用C++11的静态局部变量保证延迟初始化。
     *
     * @return GlobalTextureManager& 全局唯一实例的引用
     */
    static GlobalTextureManager& getInstance();

    // 禁用拷贝构造和赋值操作符（单例模式要求）
    GlobalTextureManager(const GlobalTextureManager&) = delete;
    GlobalTextureManager& operator=(const GlobalTextureManager&) = delete;

    /**
     * @brief 初始化纹理管理器
     *
     * 必须在OpenGL上下文创建后调用，用于获取OpenGL能力信息
     *
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool initialize();

    /**
     * @brief 从文件路径加载纹理
     *
     * @param filePath 图片文件路径（支持相对路径和绝对路径）
     * @param textureKey 纹理的唯一标识符（可选，默认使用文件路径）
     * @param generateMipmap 是否生成Mipmap（默认true）
     * @return true 加载成功
     * @return false 加载失败
     */
    bool loadTexture(const std::string& filePath,
                    const std::string& textureKey = "",
                    bool generateMipmap = true);

    /**
     * @brief 将纹理绑定到Shader uniform变量
     *
     * @param textureKey 纹理标识符
     * @param programId Shader程序ID
     * @param uniformName uniform变量名
     * @param textureUnit 纹理单元（默认自动分配）
     * @return true 绑定成功
     * @return false 绑定失败
     */
    bool bindToShader(const std::string& textureKey,
                     GLuint programId,
                     const std::string& uniformName,
                     GLint textureUnit = -1);

    /**
     * @brief 在渲染循环中激活已绑定的纹理
     *
     * 该方法应在每帧渲染时调用，用于激活所有已绑定的纹理
     */
    void activateTextures();

    /**
     * @brief 获取纹理信息
     *
     * @param textureKey 纹理标识符
     * @return const TextureInfo* 纹理信息指针，如果不存在返回nullptr
     */
    const TextureInfo* getTextureInfo(const std::string& textureKey) const;

    /**
     * @brief 增加纹理引用计数
     *
     * 当某个组件开始使用纹理时调用，用于管理纹理生命周期
     *
     * @param textureKey 纹理标识符
     * @return true 操作成功
     * @return false 纹理不存在
     */
    bool addReference(const std::string& textureKey);

    /**
     * @brief 减少纹理引用计数
     *
     * 当某个组件不再使用纹理时调用，引用计数为0时自动释放纹理
     *
     * @param textureKey 纹理标识符
     * @return true 操作成功
     * @return false 纹理不存在
     */
    bool removeReference(const std::string& textureKey);

    /**
     * @brief 强制移除纹理
     *
     * 无视引用计数，强制删除纹理资源
     *
     * @param textureKey 纹理标识符
     * @return true 移除成功
     * @return false 纹理不存在
     */
    bool forceRemoveTexture(const std::string& textureKey);

    /**
     * @brief 清理所有纹理资源
     *
     * 通常在程序退出时调用，释放所有OpenGL纹理资源
     */
    void cleanup();

    /**
     * @brief 获取已加载的纹理数量
     */
    size_t getTextureCount() const;

    /**
     * @brief 检查纹理是否存在
     */
    bool hasTexture(const std::string& textureKey) const;

    /**
     * @brief 获取所有纹理的键名列表
     */
    std::vector<std::string> getTextureKeys() const;

    /**
     * @brief 设置默认的纹理参数
     *
     * @param wrapS S轴包装模式
     * @param wrapT T轴包装模式
     * @param minFilter 缩小过滤模式
     * @param magFilter 放大过滤模式
     */
    void setDefaultTextureParameters(GLenum wrapS = GL_REPEAT,
                                   GLenum wrapT = GL_REPEAT,
                                   GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR,
                                   GLenum magFilter = GL_LINEAR);

    /**
     * @brief 获取纹理使用统计信息
     *
     * @return std::string 格式化的统计信息
     */
    std::string getUsageStatistics() const;

private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    GlobalTextureManager() = default;

    /**
     * @brief 析构函数 - 自动清理所有纹理资源
     */
    ~GlobalTextureManager();

    /**
     * @brief 从文件加载图片数据
     *
     * @param filePath 文件路径
     * @param width 输出宽度
     * @param height 输出高度
     * @param channels 输出通道数
     * @return unsigned char* 图片数据指针，使用后需要释放
     */
    unsigned char* loadImageData(const std::string& filePath,
                                int& width,
                                int& height,
                                int& channels);

    /**
     * @brief 释放图片数据
     */
    void freeImageData(unsigned char* data);

    /**
     * @brief 创建OpenGL纹理
     *
     * @param data 图片数据
     * @param width 宽度
     * @param height 高度
     * @param channels 通道数
     * @param generateMipmap 是否生成Mipmap
     * @return GLuint 纹理ID，0表示失败
     */
    GLuint createGLTexture(unsigned char* data,
                          int width,
                          int height,
                          int channels,
                          bool generateMipmap);

    /**
     * @brief 根据通道数确定OpenGL格式
     */
    GLenum getGLFormat(int channels) const;

    /**
     * @brief 生成纹理键名（如果未提供）
     */
    std::string generateTextureKey(const std::string& filePath) const;

    /**
     * @brief 分配下一个可用的纹理单元
     */
    GLint allocateTextureUnit();

    /**
     * @brief 验证文件路径
     */
    bool validateFilePath(const std::string& filePath) const;

    /**
     * @brief 内部移除纹理实现（不加锁）
     */
    bool removeTextureInternal(const std::string& textureKey);

private:

    // 纹理存储
    std::unordered_map<std::string, std::unique_ptr<TextureInfo>> m_textures;

    // Shader绑定信息
    std::unordered_map<std::string, std::vector<ShaderBinding>> m_shaderBindings;

    // 纹理单元分配器
    GLint m_nextTextureUnit = 0;

    // 默认纹理参数
    GLenum m_defaultWrapS = GL_REPEAT;
    GLenum m_defaultWrapT = GL_REPEAT;
    GLenum m_defaultMinFilter = GL_LINEAR_MIPMAP_LINEAR;
    GLenum m_defaultMagFilter = GL_LINEAR;

    // 最大纹理单元数
    GLint m_maxTextureUnits = 0;

    // 是否已初始化
    bool m_initialized = false;
};
