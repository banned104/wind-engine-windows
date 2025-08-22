#pragma once
#include <GLES3/gl3.h>

class UniformBuffer {
public:
    /**
     * @brief 构造一个 Uniform Buffer 对象
     * @param size 缓冲区的大小（字节），通常是 sizeof(YourUboStruct)
     * @param bindingPoint 要绑定的全局绑定点索引
     */
    UniformBuffer(GLsizeiptr size, GLuint bindingPoint);

    /**
     * @brief 析构函数，自动释放 OpenGL 缓冲区资源
     */
    ~UniformBuffer();

    // 删除拷贝构造和拷贝赋值，因为 OpenGL 资源不应该被轻易拷贝
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    // 允许移动构造和移动赋值
    UniformBuffer(UniformBuffer&& other) noexcept;
    UniformBuffer& operator=(UniformBuffer&& other) noexcept;

    /**
     * @brief 更新整个缓冲区的数据
     * @param data 指向要上传数据的指针
     * @param size 要上传数据的大小（字节）
     */
    void SetData(const void* data, GLsizeiptr size);

    /**
     * @brief 更新缓冲区的一部分数据
     * @param data 指向要上传数据的指针
     * @param offset 缓冲区内的偏移量（字节）
     * @param size 要上传数据的大小（字节）
     */
    void SetSubData(const void* data, GLintptr offset, GLsizeiptr size);

    /**
     * @brief 将此 UBO 绑定到其指定的全局绑定点
     */
    void Bind() const;

    /**
     * @brief 解绑当前 GL_UNIFORM_BUFFER 目标
     */
    void Unbind() const;

    /**
     * @brief 将Shader中找到的block的位置绑定到 当前实例的全局绑定点
     * [ program block pos ] -> [ m_bindingPonits ]  <- [ m_uboID ] 全局绑定点和UBO ID 如此关联
    */
    void BindShaderBolckToGlobalBindingPoint( GLuint program, GLuint shader_block_pos  );

private:
    GLuint m_uboId = 0;
    GLuint m_bindingPoint = 0;
};