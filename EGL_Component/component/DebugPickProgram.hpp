#pragma once
#include "ShaderProgram.hpp"

class DebugPickProgram : public ShaderProgram {
public:
    DebugPickProgram()
    : ShaderProgram(R"(#version 330 core
        precision mediump float;
        layout(location=0) in vec2 aPos;
        layout(location=1) in vec2 aUV;
        out vec2 vUV;
        void main(){
            vUV = aUV;
            gl_Position = vec4(aPos, 0.0, 1.0);
        })",
        R"(#version 330 core
        precision mediump float;
        uniform highp usampler2D uPick;   // 整数纹理
        in vec2 vUV;
        out vec4 FragColor;
        vec3 id2rgb(uint id){
            return  id==1u ? vec3(1,0,0) :
                    id==2u ? vec3(0,1,0) :
                    id==3u ? vec3(0,0,1) :
                    id==4u ? vec3(1,1,0) : vec3(0);
        }
        void main(){
            uint id = texture(uPick, vUV).r;   // 读取 R 通道
            vec3 col = id2rgb(id);
            float a  = id==0u ? 0.0 : 0.7;
            FragColor = vec4(col, a);
        })")
    {

        constexpr float quad[24] = {
            // pos      // uv
            0,0,  0,0,
            1,0,  1,0,
            1,1,  1,1,
            0,0,  0,0,
            1,1,  1,1,
            0,1,  0,1
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);

    }

    void drawPickDebug(int width, int height, GLuint tex)
    {

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        use();
        // 100×100 → NDC 计算
        float w = 100.0f / width;
        float h = 100.0f / height;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(glGetUniformLocation(handle(), "uPick"), 0);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

private:
    GLuint quadVAO = 0, quadVBO = 0;
};
