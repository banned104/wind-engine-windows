#pragma once

#include <glad/glad.h>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include "stb_image.h"
#include "SkyBoxShader.hpp"
#include "macros.h"

class Skybox
{
  public:
    Skybox(std::string model_dir)
    {
        std::vector<std::string> faces
        {
            model_dir+"/skybox/"+"right.jpg",
            model_dir+"/skybox/"+"left.jpg",
            model_dir+"/skybox/"+"top.jpg",
            model_dir+"/skybox/"+"bottom.jpg",
            model_dir+"/skybox/"+"front.jpg",
            model_dir+"/skybox/"+"back.jpg"
        };
        mShader = std::make_unique<SkyBoxShader>();
        // 加载 cube texture的+X,-X,+Y,-Y,+Z,-Z方向的6个面
        loadCubemap(faces);
        // 加载 skybox 的顶点、顶点纹理坐标信息，设置 VAO, VBO
        setupMesh();
    };

    void Draw( glm::mat4& view, glm::mat4& projection )
    {
        glDepthFunc(GL_LEQUAL);     // change depth function so depth test passes when values are equal to depth buffer's content
        mShader->use();
        // gl_Position = projection * view * vec4(aPos, 1.0);
        mShader->setMat4("projection", projection);
        view = glm::mat4(glm::mat3(view)); 
        mShader->setMat4("view", view);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default
    }

  private:
    // render data
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int cubemapTexture;
    const float skyboxVertices[108] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    std::unique_ptr<SkyBoxShader> mShader;

    void loadCubemap(std::vector<std::string> faces)
    {
        
        // 生成一个 cube map 纹理
        glGenTextures(1, &cubemapTexture);
        // 绑定该纹理
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        int width, height, nrChannels;
        // 加载纹理图片
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            // right(+X), left(-X), top(+Y), bottom(-Y), front(+Z), back(-Z)
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                             data);
                stbi_image_free(data);
            }
            else
            {
                // std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                LOGI("Cubemap texture failed to load at path: %s", faces[i].c_str());
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    }
};
