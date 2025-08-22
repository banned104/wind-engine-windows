#pragma once

#include "ShaderProgram.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "macros.h"

class ModelProgram : public ShaderProgram {
public:
    // UBO 的绑定点
    static constexpr GLuint BINDING_GLOBALS = 0;

    ModelProgram()
    : ShaderProgram(kVertexSrc, kFragmentSrc)
    {
        GLuint program = handle();

        // 显式绑定 uniform block
        GLuint globalsIndex = glGetUniformBlockIndex(program, "Globals");
        if (globalsIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(program, globalsIndex, BINDING_GLOBALS);
        }

        // 创建并绑定 UBO
        glGenBuffers(1, &uboGlobals);
        glBindBuffer(GL_UNIFORM_BUFFER, uboGlobals);
        // 为 UBO 分配内存，使用 GL_DYNAMIC_DRAW 因为 MVP 矩阵可能会每帧更新
        glBufferData(GL_UNIFORM_BUFFER, sizeof(GlobalsUBO), nullptr, GL_DYNAMIC_DRAW);
        // 将 UBO 绑定到指定的绑定点
        glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_GLOBALS, uboGlobals);
        glBindBuffer(GL_UNIFORM_BUFFER, 0); // 解绑
    }

    ~ModelProgram() {
        glDeleteBuffers(1, &uboGlobals);
    }

    // UBO 结构体，只包含渲染单个模型所需的基本矩阵
    struct GlobalsUBO {
        glm::mat4 proj;
        glm::mat4 view;
        glm::mat4 model;
    };

    // 更新 UBO 内容的函数
    void updateGlobals(const GlobalsUBO& g) {
        glBindBuffer(GL_UNIFORM_BUFFER, uboGlobals);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlobalsUBO), &g);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    // 提供一个简单的 use() 方法，方便在 ModelRenderer 中调用
    void use() const {
        glUseProgram(handle());
    }

    // 提供一个方法获取 Program ID，方便 ModelRenderer 设置纹理 uniform
    GLuint getProgramId() const {
        return handle();
    }

private:
    GLuint uboGlobals{};

    /* ---- 简化的 GLSL 着色器代码 ---- */

    // 顶点着色器：只做最基本的 MVP 变换
    static constexpr const char* kVertexSrc = R"(#version 300 es
        precision mediump float;

        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aTexCoords;

        // UBO: 包含 MVP 矩阵
        layout(std140) uniform Globals {
            mat4 uProj;
            mat4 uView;
            mat4 uModel;
        };

        out vec2 TexCoords;

        void main() {
            gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
            TexCoords = aTexCoords;
        }
    )";

    // 片段着色器：只做最基本的纹理采样
    static constexpr const char* kFragmentSrc = R"(#version 300 es
        precision mediump float;

        in vec2 TexCoords;
        out vec4 FragColor;

        // 材质结构体，包含一个漫反射纹理
        // 注意：这里的 uniform 命名 "material.texture_diffuse1"
        // 需要和 ModelLoader::Draw 中拼接的字符串一致
        struct Material {
            sampler2D texture_diffuse1;
        };
        uniform Material material;

        void main() {
            FragColor = texture(material.texture_diffuse1, TexCoords);
        }
    )";
};
