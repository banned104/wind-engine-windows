#pragma once

#include "ShaderProgram.hpp"

#include "wind.vert.h"
#include "wind.frag.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>

#include "macros.h"

#ifndef __COMPLEX_MODEL__
#define __COMPLEX_MODEL__
#endif
// UBO 结构体，只包含渲染单个模型所需的基本矩阵


class ModelProgram : public ShaderProgram {
public:
    struct WindUBO {
        glm::mat4 proj;
        glm::mat4 view;
        glm::mat4 model;

        float time;
        float waveAmp;
        float waveSpeed;
        int pickedInstanceID;   // use for picked instance, -1 means no picked instance

        glm::vec4 color;

        glm::vec3 boundMin;
        float deltaX;           // 保留单一deltaX用于兼容性
        glm::vec3 boundMax;
        float deltaY;           // 保留单一deltaY用于兼容性

        // 每个实例的独立偏移数组
        InstanceOffset instanceOffsets[INSTANCES_COUNT];
    };
    // UBO 的绑定点
    static constexpr GLuint BINDING_GLOBALS = 0;

    ModelProgram()
    : ShaderProgram(WIND_VERTEX_SHADER, WIND_FRAGMENT_SHADER)
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
        glBufferData(GL_UNIFORM_BUFFER, sizeof(WindUBO), nullptr, GL_DYNAMIC_DRAW);
        // 将 UBO 绑定到指定的绑定点
        glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_GLOBALS, uboGlobals);
        glBindBuffer(GL_UNIFORM_BUFFER, 0); // 解绑
    }

    ~ModelProgram() {
        glDeleteBuffers(1, &uboGlobals);
    }


    // 更新 UBO 内容的函数
    void updateGlobals(const WindUBO& g) {
        glBindBuffer(GL_UNIFORM_BUFFER, uboGlobals);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(WindUBO), &g);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    /*
     *@brief: 更新 UBO 的一部分数据
     *@param: 懂的都懂
     *@usage: 作为函数指针传入CameraInteractor
     */
    void updateGlobalsPartDelta( float input_deltaX, float input_deltaY ) {
        // WindUBO temp_g;
        // temp_g.deltaX = input_deltaX;
        // temp_g.deltaY = input_deltaY;
        // updateGlobals(temp_g);
        glBindBuffer( GL_UNIFORM_BUFFER, uboGlobals );
        glBufferSubData( GL_UNIFORM_BUFFER, offsetof( WindUBO, deltaX ) , sizeof(float), &input_deltaX );
        glBufferSubData( GL_UNIFORM_BUFFER, offsetof( WindUBO, deltaY ) , sizeof(float), &input_deltaY );
        glBindBuffer( GL_UNIFORM_BUFFER, 0 );
    }

    // 提供一个简单的 use() 方法，方便在 ModelRenderer 中调用
    void use() const {
        glUseProgram(handle());
    }

    // 提供一个方法获取 Program ID，方便 ModelRenderer 设置纹理 uniform
    GLuint getProgramId() const {
        return handle();
    }
    

    void cacheUniformLocations() {
        m_uniformLocations["texture_diffuse1"] = glGetUniformLocation(handle(), "material.texture_diffuse1");
        m_uniformLocations["texture_diffuse2"] = glGetUniformLocation(handle(), "material.texture_diffuse2");
        m_uniformLocations["texture_diffuse3"] = glGetUniformLocation(handle(), "material.texture_diffuse3");
    }

    // 使用变量位置缓存 设置Shader中保存观察者位置的变量
    void setViewPos( const glm::vec3& pos ) {
        glUniform3fv( m_uniformLocations["viewPos"], 1, glm::value_ptr( pos ) );
    }
    // 设置纹理单元
    void setTexture( unsigned int unit, const std::string& name  ) {
        if ( m_uniformLocations.find( name ) != m_uniformLocations.end() ) {
            glUniform1i( m_uniformLocations[name], unit );
        }
    }





private:

    #ifdef __COMPLEX_MODEL__
    std::unordered_map<std::string, GLint> m_uniformLocations;

    #endif

    GLuint uboGlobals{};

};