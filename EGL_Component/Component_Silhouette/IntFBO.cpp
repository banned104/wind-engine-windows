
#include "IntFBO.hpp"
#include "macros.h"


/**/


/**/


IntFBO::IntFBO() : fbo(0), rbo(0), tex(0)
{

}

IntFBO::IntFBO(int width, int height, GLint internalFormat, GLenum format, GLenum type)
{
    glGenFramebuffers(1, &fbo);
    if (fbo == 0) {
        throw std::runtime_error("Failed to generate FBO");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // --- 颜色附件 (Color Attachment) ---
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // *** 这是最关键的一步 ***
    // 使用你传入的参数来定义纹理的存储格式
    // 注意：最后一个参数是 nullptr，因为我们只是分配空间，不上传初始数据
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);

    // 对于整数纹理，必须使用 NEAREST 采样，否则行为未定义
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // 将纹理附加到 FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    // --- 深度附件 (Depth Attachment) ---
    // 对于拾取渲染，深度附件是必需的，以确保正确的遮挡关系
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // 检查 FBO 是否完整
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &tex);
        glDeleteRenderbuffers(1, &rbo);
        fbo = 0;
        tex = 0;
        rbo = 0;
        char msg[256];
        sprintf(msg, "Framebuffer is not complete! Status: 0x%x", status);
        throw std::runtime_error(msg);
    }

    // 解绑，恢复默认状态
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// 错误❌
//IntFBO::~IntFBO()
//{
//	GLES_CHECK_ERROR(glDeleteFramebuffers(1, &fbo));
//	GLES_CHECK_ERROR(glDeleteRenderbuffers(1, &rbo));
//	GLES_CHECK_ERROR(glDeleteTextures(1, &tex));
//	fbo = 0;
//    rbo = 0;
//    tex = 0;
//}

IntFBO::~IntFBO()
{
    // 检查当前是否有有效的OpenGL上下文
    if (glfwGetCurrentContext() != nullptr) {
        // 检查OpenGL对象是否有效再删除
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
            fbo = 0;
        }

        if (rbo != 0) {
            glDeleteRenderbuffers(1, &rbo);
            rbo = 0;
        }

        if (tex != 0) {
            glDeleteTextures(1, &tex);
            tex = 0;
        }
    }
}

void IntFBO::bind()
{
    if (fbo == 0) {
        throw std::runtime_error("Attempted to bind an invalid FBO (ID is 0)");
    }
    // Clear any outstanding errors before our call
    while(glGetError() != GL_NO_ERROR);

    // LOGI("Binding FBO with ID: %d", fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Check for errors specifically from this operation
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        char msg[256];
        sprintf(msg, "Error binding FBO ID %d: 0x%x", fbo, err);
        LOGE("%s", msg);
        throw std::runtime_error(msg);
    }
}

void IntFBO::unbind()
{
	GLES_CHECK_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

GLuint IntFBO::getFBO()
{
	return fbo;
}

GLuint IntFBO::getTex()
{
	return tex;
}