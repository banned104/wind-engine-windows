#pragma once
#include "ShaderProgram.hpp"
#include "LyFBO.h"
#include "UniformBuffer.hpp"
#include "GlobalBindingPoints.hpp"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <memory>

/*
    根据3D模型轮廓变化以改变鼠标触控区域变化
*/
#include "CommonTypes.hpp"

class SilhouettesClass : public ShaderProgram {
public:
    SilhouettesClass() : ShaderProgram(vertex_shader, frag_shader) {
        blockIndex = glGetUniformBlockIndex(handle(), "Globals");
        m_uboClass = std::make_unique<UniformBuffer>( sizeof( Globals), UboBindingPoints::Globals );
        // 绑定SHader中的Block到全局绑定点中 -> UboBindingPoints::Globals
        m_uboClass->BindShaderBolckToGlobalBindingPoint( handle(), blockIndex );
    }

    ~SilhouettesClass() {/* 什么都不用写 用智能指针就好 */}

    // 每一帧循环的时候都要Bind一次UBO
    void updateUBOData( const Globals& g ) {
        m_uboClass->SetData( &g, sizeof( Globals ) );
        m_uboClass->Bind();
    }

private:
    std::unique_ptr<UniformBuffer> m_uboClass;
    GLuint blockIndex;

    constexpr static const char* vertex_shader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        // layout (location = 1) in vec2 aTexCoords;
        // layout (location = 2) in vec3 aNormal;
        // layout (location = 3) in mat4 instanceTransform;

        layout(std140) uniform Globals {
            mat4 modelMatrix;
            mat4 viewMatrix;
            mat4 projMatrix;

            uint id;
        };

        out vec2 TexCoords;
        flat out uint instanceID;
        
        void main()
        {
            instanceID = id;
            // 先计算变换后的位置
            vec4 worldPos = modelMatrix * vec4(aPos, 1.0);
            // 正常变换
            gl_Position = projMatrix * viewMatrix * worldPos;
        }
    )";

    constexpr static const char* frag_shader = R"(
        #version 330 core
        precision mediump float;
        layout( location = 0 ) out uint out_id;

        flat in uint instanceID;

        void main()
        {
            out_id = instanceID;
        }
    )";
};