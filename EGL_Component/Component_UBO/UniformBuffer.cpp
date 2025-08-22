#include "UniformBuffer.hpp"
#include <utility> // For std::swap


/*
Example:
    * 在你的 LoadingViewClass 或 ModelProgram 的构造函数中
    * 先找到着色器里那个叫 "Globals" 的 Uniform Block 的内部索引
        GLuint blockIndex = glGetUniformBlockIndex(programId, "Globals");

    * 告诉这个着色器程序：“你的 'Globals' 这个 Uniform Block，应该去全局的 0 号插座取数据。”
        glUniformBlockBinding(programId, blockIndex, 0); // 这里的 0 就是 m_bindingPoint

    * 这样 这个着色器的 Globals 块就会绑定到 0 这个 UBO的全局绑定点
    * 但是这个 0 是魔法数字, 还需要创建一个全局的UBO绑定点约定 --> "GlobalBindingPoints.hpp"
    * 在其他文件使用UBO时不仅要包含本类 还需要这个全局绑定点的约定
*/


UniformBuffer::UniformBuffer(GLsizeiptr size, GLuint bindingPoint)
    : m_bindingPoint(bindingPoint) {
    glGenBuffers(1, &m_uboId);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboId);
    // 分配内存，并指定为 DYNAMIC_DRAW 以便后续更新
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // 将缓冲区绑定到指定的全局绑定点
    glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_uboId);
}

UniformBuffer::~UniformBuffer() {
    if (m_uboId != 0) {
        glDeleteBuffers(1, &m_uboId);
    }
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : m_uboId(other.m_uboId), m_bindingPoint(other.m_bindingPoint) {
    other.m_uboId = 0; // 防止旧对象在析构时删除缓冲区
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept {
    if (this != &other) {
        if (m_uboId != 0) {
            glDeleteBuffers(1, &m_uboId);
        }
        m_uboId = other.m_uboId;
        m_bindingPoint = other.m_bindingPoint;
        other.m_uboId = 0;
    }
    return *this;
}

void UniformBuffer::SetData(const void* data, GLsizeiptr size) {
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboId);
    glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

/* 部分数据设置 */
void UniformBuffer::SetSubData(const void* data, GLintptr offset, GLsizeiptr size) {
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboId);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::Bind() const {
    glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_uboId);
}

void UniformBuffer::Unbind() const {
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void UniformBuffer::BindShaderBolckToGlobalBindingPoint( GLuint program, GLuint shader_block_pos ) {
    if ( shader_block_pos != GL_INVALID_INDEX ) {
        glUniformBlockBinding( program, shader_block_pos, m_bindingPoint );
    } else { while (1); }
/*
    对于这个函数 AI不推荐这样写 应该如下方式
    // ... (includes) ...

class SilhouettesClass : public ShaderProgram {
public:
    SilhouettesClass() 
        : ShaderProgram(vertex_shader, frag_shader) // 1. 基类构造，创建了 Program
    {
        // 2. 创建 UBO 实例。它在构造时会自动把自己插入到 UboBindingPoints::Globals 插座
        m_uboClass = std::make_unique<UniformBuffer>(sizeof(Globals), UboBindingPoints::Globals);

        // 3. 配置自己！告诉自己的 "Globals" block，去 UboBindingPoints::Globals 插座上收听数据
        GLuint blockIndex = glGetUniformBlockIndex(handle(), "Globals");
        if (blockIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(handle(), blockIndex, UboBindingPoints::Globals);
        }
    }

    // ... (其他代码) ...
};
*/



}