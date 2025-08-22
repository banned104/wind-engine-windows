#include "BoundingBoxRenderer.hpp"

BoundingBoxRenderer::BoundingBoxRenderer() {
    // 构造函数中不进行OpenGL操作，因为可能还没有OpenGL上下文
}

BoundingBoxRenderer::~BoundingBoxRenderer() {
    cleanup();
}

bool BoundingBoxRenderer::initialize() {
    if (mInitialized) {
        return true;
    }
    
    try {
        // 检查OpenGL上下文是否有效
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            LOGE("OpenGL error before BoundingBoxRenderer initialization: 0x%x", error);
        }
        
        // 创建着色器程序
        mProgram = std::make_unique<BoundingBoxProgram>();
        if (!mProgram || mProgram->handle() == 0) {
            LOGE("Failed to create BoundingBoxProgram");
            return false;
        }
        
        // 创建包围盒几何体
        createBoundingBoxGeometry();
        
        mInitialized = true;
        LOGI("BoundingBoxRenderer initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOGE("Failed to initialize BoundingBoxRenderer: %s", e.what());
        cleanup();
        return false;
    }
}

void BoundingBoxRenderer::createBoundingBoxGeometry() {
    // 包围盒的8个顶点（单位立方体，后续通过变换调整到实际大小）
    float vertices[VERTEX_COUNT * 3] = {
        // 底面4个顶点
        0.0f, 0.0f, 0.0f,  // 0: 左下后
        1.0f, 0.0f, 0.0f,  // 1: 右下后
        1.0f, 0.0f, 1.0f,  // 2: 右下前
        0.0f, 0.0f, 1.0f,  // 3: 左下前
        
        // 顶面4个顶点
        0.0f, 1.0f, 0.0f,  // 4: 左上后
        1.0f, 1.0f, 0.0f,  // 5: 右上后
        1.0f, 1.0f, 1.0f,  // 6: 右上前
        0.0f, 1.0f, 1.0f   // 7: 左上前
    };
    
    // 包围盒的12条边的索引（每条边用2个顶点表示）
    unsigned int indices[INDEX_COUNT] = {
        // 底面4条边
        0, 1,  1, 2,  2, 3,  3, 0,
        // 顶面4条边
        4, 5,  5, 6,  6, 7,  7, 4,
        // 垂直4条边
        0, 4,  1, 5,  2, 6,  3, 7
    };
    
    // 清除之前的错误状态
    while (glGetError() != GL_NO_ERROR) {}
    
    // 生成并绑定VAO
    glGenVertexArrays(1, &mVAO);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after glGenVertexArrays: 0x%x", error);
        throw std::runtime_error("Failed to generate VAO");
    }
    
    glBindVertexArray(mVAO);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after glBindVertexArray: 0x%x", error);
        throw std::runtime_error("Failed to bind VAO");
    }
    
    // 生成并设置VBO
    glGenBuffers(1, &mVBO);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after glGenBuffers(VBO): 0x%x", error);
        throw std::runtime_error("Failed to generate VBO");
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after glBindBuffer(VBO): 0x%x", error);
        throw std::runtime_error("Failed to bind VBO");
    }
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after glBufferData(VBO): 0x%x", error);
        throw std::runtime_error("Failed to upload VBO data");
    }
    
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after glVertexAttribPointer: 0x%x", error);
        throw std::runtime_error("Failed to set vertex attribute pointer");
    }
    
    glEnableVertexAttribArray(0);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after glEnableVertexAttribArray: 0x%x", error);
        throw std::runtime_error("Failed to enable vertex attribute array");
    }
    
    // 生成并设置EBO
    glGenBuffers(1, &mEBO);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after glGenBuffers(EBO): 0x%x", error);
        throw std::runtime_error("Failed to generate EBO");
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after glBindBuffer(EBO): 0x%x", error);
        throw std::runtime_error("Failed to bind EBO");
    }
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after glBufferData(EBO): 0x%x", error);
        throw std::runtime_error("Failed to upload EBO data");
    }
    
    // 解绑VAO
    glBindVertexArray(0);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("OpenGL error after unbinding VAO: 0x%x", error);
        throw std::runtime_error("Failed to unbind VAO");
    }
    
    LOGI("BoundingBox geometry created successfully");
}

void BoundingBoxRenderer::drawBoundingBox(const glm::vec3& minBounds, 
                                        const glm::vec3& maxBounds,
                                        const glm::mat4& mvpMatrix,
                                        const glm::vec3& color) {
    if (!mInitialized) {
        LOGE("BoundingBoxRenderer not initialized");
        return;
    }
    
    // 保存当前OpenGL状态
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    
    // 计算包围盒的变换矩阵
    // 我们的单位立方体顶点范围是[0,1]，需要变换到[minBounds, maxBounds]
    glm::vec3 size = maxBounds - minBounds;
    glm::vec3 center = (minBounds + maxBounds) * 0.5f;
    
    // 先缩放到正确大小，再平移到正确位置
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), size);
    glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), minBounds);
    glm::mat4 modelMatrix = translateMatrix * scaleMatrix;
    glm::mat4 finalMVP = mvpMatrix * modelMatrix;
    
    // 启用深度测试但禁用深度写入，这样线框不会被模型遮挡但也不会影响其他物体
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    // 启用线框模式的混合
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 使用着色器程序
    mProgram->use();
    mProgram->setMVP(finalMVP);
    mProgram->setColor(color);
    
    // 绑定VAO并绘制
    glBindVertexArray(mVAO);
    glDrawElements(GL_LINES, INDEX_COUNT, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // 恢复OpenGL状态
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
    if (!depthTestEnabled) {
        glDisable(GL_DEPTH_TEST);
    }
    
    glUseProgram(currentProgram);
}

void BoundingBoxRenderer::cleanup() {
    if (mVAO != 0) {
        glDeleteVertexArrays(1, &mVAO);
        mVAO = 0;
    }
    
    if (mVBO != 0) {
        glDeleteBuffers(1, &mVBO);
        mVBO = 0;
    }
    
    if (mEBO != 0) {
        glDeleteBuffers(1, &mEBO);
        mEBO = 0;
    }
    
    mProgram.reset();
    mInitialized = false;
    
    LOGI("BoundingBoxRenderer cleaned up");
}