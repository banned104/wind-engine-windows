#pragma once

#include "ShaderProgram.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>

#include "macros.h"

#ifndef __COMPLEX_MODEL__
#define __COMPLEX_MODEL__
#endif

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
    
    #ifdef __COMPLEX_MODEL__
    void cacheUniformLocations() {
        m_uniformLocations["light.position"] = glGetUniformLocation(handle(), "light.position");
        m_uniformLocations["light.color"]    = glGetUniformLocation(handle(), "light.color");
        m_uniformLocations["viewPos"]        = glGetUniformLocation(handle(), "viewPos");
        m_uniformLocations["material.shininess"] = glGetUniformLocation(handle(), "material.shininess");
    }

    // 使用变量位置缓存 设置Shader中保存光照位置/颜色的变量
    void setLight(const glm::vec3& pos, const glm::vec3& color) {
        glUniform3fv(m_uniformLocations["light.position"], 1, glm::value_ptr(pos));
        glUniform3fv(m_uniformLocations["light.color"], 1, glm::value_ptr(color));
    }

    // 使用变量位置缓存 设置Shader中保存观察者位置的变量
    void setViewPos( const glm::vec3& pos ) {
        glUniform3fv( m_uniformLocations["viewPos"], 1, glm::value_ptr( pos ) );
    }

    void setShininess( const GLfloat shine ) {
        glUniform1f( m_uniformLocations["material.shininess"], shine );
    }
    #endif



private:

    #ifdef __COMPLEX_MODEL__
    std::unordered_map<std::string, GLint> m_uniformLocations;

    #endif

    GLuint uboGlobals{};

    /* ---- 升级后的 GLSL 着色器代码 ---- */

    // 顶点着色器：扩展，以传递更多信息给片段着色器
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
        layout (location = 5) in mat4 aInstanceMatrix;
        layout (location = 9) in vec4 aInstanceColor;

        // 输出到片段着色器
        out vec3 FragPos;   // 片段在世界空间中的位置
        out vec3 Normal;    // 片段在世界空间中的法线
        out vec2 TexCoords; // 纹理坐标
        out vec4 InstanceColor;

        void main() {
            // 将顶点位置和法线变换到世界空间
            FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));
            // 注意：法线需要用逆转置矩阵变换以避免非等比缩放导致的问题
            // 为简化，这里我们假设 uModel 没有非等比缩放
            Normal = mat3(transpose(inverse(aInstanceMatrix))) * aNormal;
            
            TexCoords = aTexCoords;
            InstanceColor = aInstanceColor;
            gl_Position = uProj * uView * vec4(FragPos, 1.0);
        }
    )";

    // 片段着色器：实现 Phong 光照模型
    static constexpr const char* kFragmentSrc = R"(#version 300 es
        precision mediump float;

        // 从顶点着色器传入的、经过插值的数据
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoords;
        in vec4 InstanceColor;

        out vec4 FragColor;

        // 材质结构体，现在包含多种纹理
        struct Material {
            sampler2D texture_diffuse1;
            sampler2D texture_specular1;
            // sampler2D texture_normal1; // 法线贴图暂时注释，待后续实现
            float shininess;
        };
        uniform Material material;

        // 光源结构体
        struct Light {
            vec3 position; // 在世界空间中的位置
            vec3 color;
        };
        uniform Light light;

        // 观察者（相机）位置
        uniform vec3 viewPos;

        void main() {
            // 1. 环境光 (Ambient)
            // 直接从漫反射贴图采样，并乘以一个较小的环境光强度
            vec3 ambient = 0.54 * texture(material.texture_diffuse1, TexCoords).rgb;

            // 2. 漫反射光 (Diffuse)
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(light.position - FragPos);
            // 计算光线和法线的夹角，max确保不会是负数
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuseColor = texture(material.texture_diffuse1, TexCoords).rgb;
            vec3 diffuse = light.color * diff * diffuseColor;

            // 3. 镜面光 (Specular)
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            // 计算反射光和视线的夹角，并用 shininess 控制高光大小
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
            // 从高光贴图采样，决定镜面反射的强度和颜色
            vec3 specularColor = texture(material.texture_specular1, TexCoords).rgb;
            vec3 specular = light.color * spec * specularColor;

            // 最终颜色 = 环境光 + 漫反射光 + 镜面光
            vec3 result = ambient + diffuse + specular;
            FragColor = vec4(result, 1.0);
        }
    )";
};