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

    #ifdef __ANDROID__
    constexpr static const char* vertex_shader = R"(
        #version 310 es
        precision mediump float;
        #define INSTANCES_COUNT 4
        layout (location = 0) in vec3 aPos;
        layout (location = 2) in vec2 aTexCoords;
        // layout (location = 2) in vec3 aNormal;
        // layout (location = 3) in mat4 instanceTransform;
        layout (location = 5) in mat4 aInstanceMatrix;
        layout (location = 9) in uint aInstanceId;

        layout(std140) uniform Globals {
            mat4 modelMatrix;
            mat4 viewMatrix;
            mat4 projMatrix;

            // 每个实例的独立偏移数组
            vec4 InstanceOffset[ INSTANCES_COUNT ];

            // 新增字段
            int pickedInstanceID;
            float deltaX;
            float deltaY;
            float _padding;  // 对齐
            
            uint id;
        };

        // 纹理在OpenGL中是全局加载 所以此处可以直接读取
        uniform sampler2D vertexMovementTexture;

        
        flat out uint instanceID;
        
        void main()
        {
            instanceID = aInstanceId;
            vec2 TexCoords = aTexCoords;
            vec3 FragPos;
            // 将顶点位置和法线变换到世界空间
            FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));

            // // 应用每个实例的独立偏移
            // int instanceIndex = int(aInstanceId) - 1;  // 实例ID从1开始，数组索引从0开始
            // if (instanceIndex >= 0 && instanceIndex < INSTANCES_COUNT) {
            //     // 使用该实例的独立偏移
            //     vec4 movementData = texture( vertexMovementTexture, TexCoords );
            //     float movementFactor = (movementData.r + movementData.g + movementData.b) * 0.33333333;
            //     FragPos.x += InstanceOffset[instanceIndex].x * movementFactor;
            //     FragPos.y -= InstanceOffset[instanceIndex].y * movementFactor;  // Y轴方向相反
            // }
                // 应用每个实例的独立偏移
                int instanceIndex = int(aInstanceId) - 1;
                if (instanceIndex >= 0 && instanceIndex < 4) {
                    FragPos.x += InstanceOffset[instanceIndex].x  * TexCoords.x;
                    FragPos.y -= InstanceOffset[instanceIndex].y  * TexCoords.x;
                }

            gl_Position = projMatrix * viewMatrix * vec4(FragPos, 1.0);
        }
    )";

    constexpr static const char* frag_shader = R"(
        #version 310 es
        precision mediump float;
        layout( location = 0 ) out uint out_id;

        flat in uint instanceID;

        void main()
        {
            out_id = instanceID;
        }
    )";
    #else
    constexpr static const char* vertex_shader = R"(
        #version 330 core
        #define INSTANCES_COUNT 4
        layout (location = 0) in vec3 aPos;
        layout (location = 2) in vec2 aTexCoords;
        // layout (location = 2) in vec3 aNormal;
        // layout (location = 3) in mat4 instanceTransform;
        layout (location = 5) in mat4 aInstanceMatrix;
        layout (location = 9) in uint aInstanceId;

        layout(std140) uniform Globals {
            mat4 modelMatrix;
            mat4 viewMatrix;
            mat4 projMatrix;

            // 每个实例的独立偏移数组
            vec4 InstanceOffset[ INSTANCES_COUNT ];

            // 新增字段
            int pickedInstanceID;
            float deltaX;
            float deltaY;
            float _padding;  // 对齐
            
            uint id;
        };

        // 纹理在OpenGL中是全局加载 所以此处可以直接读取
        uniform sampler2D vertexMovementTexture;

        
        flat out uint instanceID;
        
        void main()
        {
            instanceID = aInstanceId;
            vec2 TexCoords = aTexCoords;
            vec3 FragPos;
            // 将顶点位置和法线变换到世界空间
            FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));

            // // 应用每个实例的独立偏移
            // int instanceIndex = int(aInstanceId) - 1;  // 实例ID从1开始，数组索引从0开始
            // if (instanceIndex >= 0 && instanceIndex < INSTANCES_COUNT) {
            //     // 使用该实例的独立偏移
            //     vec4 movementData = texture( vertexMovementTexture, TexCoords );
            //     float movementFactor = (movementData.r + movementData.g + movementData.b) * 0.33333333;
            //     FragPos.x += InstanceOffset[instanceIndex].x * movementFactor;
            //     FragPos.y -= InstanceOffset[instanceIndex].y * movementFactor;  // Y轴方向相反
            // }
                // 应用每个实例的独立偏移
                int instanceIndex = int(aInstanceId) - 1;
                if (instanceIndex >= 0 && instanceIndex < 4) {
                    FragPos.x += InstanceOffset[instanceIndex].x  * TexCoords.x;
                    FragPos.y -= InstanceOffset[instanceIndex].y  * TexCoords.x;
                }

            gl_Position = projMatrix * viewMatrix * vec4(FragPos, 1.0);
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
    #endif
};