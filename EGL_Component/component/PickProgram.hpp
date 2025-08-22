#pragma once

#include "ShaderProgram.hpp"
#include "ModelProgram.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class PickProgram : public ShaderProgram {
public:
    static constexpr int NUM_INSTANCES = 4;
    static constexpr GLuint BINDING_GLOBALS = 0;

    PickProgram()
    : ShaderProgram(R"(#version 300 es
        precision mediump float;
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aTexCoords;
        layout(location = 3) in mat4 instanceTransform;
        #define NUM_INSTANCES 4

        layout(std140) uniform Globals {
            mat4 uModel[NUM_INSTANCES];
            mat4 uView[NUM_INSTANCES];
            mat4 uProj;

            // 原始模型的包围盒
            vec4 uBoundsMin;
            vec4 uBoundsMax;

            // waveAmp 抖动
            // waveSpeed 纵向波浪速度
            // time, waveAmp, waveSpeed 三个标量打包在一个 vec4
            // .x = time, .y = waveAmp, .z = waveSpeed, .w unused
            vec4 uParams;

            vec4 uChannelShow;

            vec4 uChannelSpeed;
        };
        out vec2 TexCoords;
        flat out int vInstanceID;
        void main() {
            vInstanceID = gl_InstanceID;
            if(uChannelShow[gl_InstanceID] < 0.5) {
                gl_Position = vec4(2.0,2.0,2.0,1.0);
                return;
            }
            TexCoords = aTexCoords;
            gl_Position = uProj *
                          instanceTransform *
                          uView[gl_InstanceID] *
                          uModel[gl_InstanceID] *
                          vec4(aPos, 1.0);
        })", 
        R"(#version 300 es
        precision mediump float;
        layout(location = 0) out lowp uint oID;   // GL_R8UI

        struct Material {
            sampler2D texture_diffuse1;   // 主体颜色
            sampler2D texture_specular1;  // 作为 mask
        };
        uniform Material material;
        flat in int vInstanceID;
        in vec2 TexCoords;
        void main() {
            float mask = texture(material.texture_specular1, TexCoords).r;
            mask = 1.0 - mask;
            if (mask < 0.2) {
                discard;
            }
            uint id = uint(vInstanceID);
            oID = id + 1u;
        })")
    {
        GLuint program = handle();
        glUseProgram(program);

        // 显式绑定 UBO
        GLuint idxG = glGetUniformBlockIndex(program, "Globals");
        if (idxG != GL_INVALID_INDEX)
            glUniformBlockBinding(program, idxG, BINDING_GLOBALS);
    }

    ~PickProgram()  {
    }

    void updateGlobals(const ModelProgram::GlobalsUBO& g, GLuint sharedUbo)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, sharedUbo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ModelProgram::GlobalsUBO), &g);
        glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_GLOBALS, sharedUbo);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void printLog(int width, int height)
    {
        std::vector<GLubyte> buf(width * height);
        glReadPixels(0, 0, width, height,
                    GL_RED_INTEGER, GL_UNSIGNED_BYTE, buf.data());
        // GLES_CHECK_ERROR(glReadPixels(0, 0, dbg, dbg,
        //             GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, buf.data()));

        for (int y = height-1; y >= 0; --y) {
            std::string line;
            for (int x = 0; x < width; ++x) {
                line += std::to_string(buf[y * width + x]) + ' ';
            }
            LOGI("%s", line.c_str());
        }
    }
};
