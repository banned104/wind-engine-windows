 #include "OffscreenRenderer.hpp"
#include "macros.h" 

OffscreenRenderer::OffscreenRenderer(int width, int height)
    : mWidth(width), mHeight(height) {
    mFbo = std::make_unique<LyFBOMSAA>(mWidth, mHeight);
    LOGI("OffscreenRenderer: MSAA FBO created with %d samples.", mFbo->getSampleCount());
    initScreenRender();
}

OffscreenRenderer::~OffscreenRenderer() {
    if (mScreenVao != 0) {
        glDeleteVertexArrays(1, &mScreenVao);
        mScreenVao = 0;
    }
    if (mScreenVbo != 0) {
        glDeleteBuffers(1, &mScreenVbo);
        mScreenVbo = 0;
    }
    // unique_ptr will handle mFbo and mScreenShader
}

void OffscreenRenderer::initScreenRender() {
    const char* screenVertexShader = R"(#version 300 es
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoords;
        out vec2 TexCoords;
        void main() {
            gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
            TexCoords = aTexCoords;
        }
    )";
    const char* screenFragmentShader = R"(#version 300 es
        precision mediump float;
        out vec4 FragColor;
        in vec2 TexCoords;
        uniform sampler2D screenTexture;
        void main() {
            FragColor = texture(screenTexture, TexCoords);
        }
    )";
    mScreenShader = std::make_unique<ShaderProgram>(screenVertexShader, screenFragmentShader);

    float quadVertices[] = { 
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &mScreenVao);
    glGenBuffers(1, &mScreenVbo);
    glBindVertexArray(mScreenVao);
    glBindBuffer(GL_ARRAY_BUFFER, mScreenVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void OffscreenRenderer::beginFrame() {
    mFbo->bindForDraw();
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, mWidth, mHeight);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OffscreenRenderer::endFrame() {
    mFbo->resolve();
}

void OffscreenRenderer::drawToScreen() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    // No need to clear here, as we are drawing a full-screen quad that will cover everything.
    // glClear(GL_COLOR_BUFFER_BIT);

    mScreenShader->use();
    glUniform1i(glGetUniformLocation(mScreenShader->handle(), "screenTexture"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mFbo->getTex());
    glBindVertexArray(mScreenVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}