#include "LyFBOMSAA.h"
#include "macros.h"

LyFBOMSAA::LyFBOMSAA(int width, int height) : LyFBO()
{
    // 1) Query max samples
    GLint maxSamples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    printf("Max samples supported: %d\n", maxSamples);

    // clamp samples to whatever you like (e.g. 4)
    GLint samples = std::min(maxSamples, 1);

    //
    // 2) Set up the multisample FBO (no texture here, just renderbuffer)
    //
    glGenFramebuffers(1, &msaaFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFbo);

    // Color renderbuffer
    glGenRenderbuffers(1, &msaaColorRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, msaaColorRbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorRbo);

    // Depth + stencil renderbuffer
    glGenRenderbuffers(1, &msaaDepthStencilRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthStencilRbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaaDepthStencilRbo);

    // Check completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("MSAA FBO incomplete: 0x%04X\n", status);
    }

    //
    // 3) Create resolve FBO with a texture
    //
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    // single sample texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    // (optional) depth/stencil on resolve FBO if you need to read depth
    // ...

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("Resolve FBO incomplete: 0x%04X\n", status);
    }

    // unbind
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // store for later
    this->width  = width;
    this->height = height;
    this->samples = samples;
}

LyFBOMSAA::~LyFBOMSAA()
{
    // delete both FBOs + their RBOs/textures
    glDeleteFramebuffers(1, &msaaFbo);
    glDeleteRenderbuffers(1, &msaaColorRbo);
    glDeleteRenderbuffers(1, &msaaDepthStencilRbo);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
}

// Call before you render your scene:
void LyFBOMSAA::bindForDraw()
{
    // render into the multisample FBO
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFbo);
    glViewport(0, 0, width, height);
}

// After rendering, call this to resolve into the texture‐backed FBO:
void LyFBOMSAA::resolve()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    // blit color buffer; you can also blit depth if needed
    glBlitFramebuffer(
        0, 0, width, height,
        0, 0, width, height,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    );
    // now 'tex' contains the resolved image
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
