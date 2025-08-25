#pragma once

#include "LyFBO.h"
#include <glad/glad.h>  // or appropriate GL include

/**
 * @file LyFBOMSAA.h
 * @brief Multisample FBO with automatic resolve into a texture.
 */
class LyFBOMSAA : public LyFBO
{
public:
    // Default constructor
    LyFBOMSAA();

    // Create an MSAA FBO of given dimensions (width x height).
    explicit LyFBOMSAA(int width, int height);

    // Destructor: cleans up FBOs, RBOs, and texture
    ~LyFBOMSAA();

    /**
     * Bind the multisample FBO for rendering.
     */
    void bindForDraw();

    /**
     * Resolve (blit) the multisampled buffer into the texture-backed FBO.
     * After calling this, the texture() from LyFBO (tex) contains the resolved image.
     */
    void resolve();

    /**
     * Get the number of samples used for MSAA.
     */
    inline int getSampleCount() const { return samples; }

private:
    // Multisample FBO and its renderbuffers
    GLuint msaaFbo       = 0;
    GLuint msaaColorRbo  = 0;
    GLuint msaaDepthStencilRbo = 0;

    // Dimensions and sample count
    int width    = 0;
    int height   = 0;
    int samples  = 0;
};
