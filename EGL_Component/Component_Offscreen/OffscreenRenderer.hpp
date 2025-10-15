#pragma once

#include <memory>

#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#else
// GLFW + GLAD
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include "../Component_FBO/LyFBOMSAA.h"
#include "component/ShaderProgram.hpp"

/**
 * @class OffscreenRenderer
 * @brief Manages off-screen rendering using an MSAA Framebuffer Object.
 *
 * This class encapsulates the creation of an FBO, a screen-sized quad,
 * and the shader program required to render the FBO's texture to the screen.
 */
class OffscreenRenderer {
public:
    /**
     * @brief Constructs the OffscreenRenderer.
     * @param width The width of the off-screen buffer.
     * @param height The height of the off-screen buffer.
     */
    OffscreenRenderer(int width, int height);

    /**
     * @brief Destructor that cleans up OpenGL resources.
     */
    ~OffscreenRenderer();

    // Disable copy and move semantics
    OffscreenRenderer(const OffscreenRenderer&) = delete;
    OffscreenRenderer& operator=(const OffscreenRenderer&) = delete;

    /**
     * @brief Binds the internal FBO to start rendering to it.
     * This should be called before rendering the main scene.
     */
    void beginFrame();

    /**
     * @brief Resolves the multisampled FBO into a texture.
     * This should be called after the main scene has been rendered to the FBO.
     */
    void endFrame();

    /**
     * @brief Renders the result (the FBO's texture) to the default framebuffer (the screen).
     */
    void drawToScreen();

private:
    void initScreenRender();

    std::unique_ptr<LyFBOMSAA> mFbo;
    std::unique_ptr<ShaderProgram> mScreenShader;
    GLuint mScreenVao = 0;
    GLuint mScreenVbo = 0; // Keep VBO handle for proper cleanup
    int mWidth;
    int mHeight;
};