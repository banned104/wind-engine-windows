#pragma once

#include "SilhouetteProgram_Instancing.hpp"
#include "IntFBO.hpp"
// #include "ModelLoader_Universal.hpp"
#include "ModelLoader_Universal_Instancing.hpp"
#include "CameraInteractor.hpp"
#include "Camera.hpp"
#include "macros.h"
#include "CommonTypes.hpp"

#include <memory>
#include <vector>


class FlexableTouchPadClass {
public:
    explicit FlexableTouchPadClass( int mWidth, int mHeight, 
        Globals& mg, 
        const Model& mModel, 
        const Camera& mCamera,
        const CameraInteractor& mInteractor
     ) :    m_height(mHeight), 
            m_width( mWidth ), 
            g( mg ), 
            mainModel( mModel ), 
            mainCamera( mCamera ), 
            mainInteractor( mInteractor )
        {
        // 初始化着色器程序 片段着色器输出 ID
        mainProgram = std::make_unique<SilhouettesClass>();
        // 初始化离屏渲染 注意后面的参数 GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT
        mainFBO = std::make_unique<IntFBO>(m_width, m_height, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT);
        
        LOGI("FlexableTouchPad created. FBO ID: %d", mainFBO->getFBO());
        if (mainFBO->getFBO() == 0) {
            LOGE("FATAL: FBO creation failed, ID is 0.");
        }
    }

    ~FlexableTouchPadClass() {}

    /**
     * @brief 执行一次拾取操作，读取触摸点下的像素ID。
     * 这是一个独立的、原子性的操作，它会保存并恢复它所修改的OpenGL状态。
     * @return 返回读取到的物体ID。如果为0，表示没有拾取到任何物体。
     */
    int performPickingInstancing() {
        // --- 1. 保存当前 OpenGL 状态 ---
        GLint last_fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
        GLint last_program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        GLint last_viewport[4];
        glGetIntegerv(GL_VIEWPORT, last_viewport);
        GLboolean last_blend_enabled = glIsEnabled(GL_BLEND);
        GLboolean last_depth_test = glIsEnabled(GL_DEPTH_TEST);

        // --- 2. 设置拾取专用的渲染环境 ---
        // Clear any existing errors before binding
        while(glGetError() != GL_NO_ERROR);

        LOGI("Attempting to bind FBO ID: %d", mainFBO->getFBO());

        try {
            mainFBO->bind();
        } catch (const std::runtime_error& e) {
            LOGE("Error binding FBO in performPicking: %s", e.what());
            return -1;
        }

        // Check for errors after binding
        GLenum post_bind_err = glGetError();
        if (post_bind_err != GL_NO_ERROR) {
            LOGE("OpenGL error after FBO bind: 0x%x", post_bind_err);
        } else {
            LOGI("FBO bound successfully");
        }
        glViewport(0, 0, m_width, m_height);
        mainProgram->use();
        if (last_blend_enabled) {
            glDisable(GL_BLEND);
        }
        glEnable(GL_DEPTH_TEST);

        // --- 3. 执行拾取绘制 ---
        // 清空FBO，背景ID为0
        GLuint clearColor = 0xFFFFFFFFu;
        glClearBufferuiv(GL_COLOR, 0, &clearColor);
        glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
        
        // 该类成员变量包含 m_globals 所以变换矩阵已经更新
        mainProgram->updateUBOData(g);
        #ifdef ENABLE_INSTANCING
        const_cast<Model&>(mainModel).DrawInstanced(mainProgram->handle(), INSTANCES_COUNT);
        #else
        mainModel.Draw(mainProgram->handle());
        #endif

        // --- 4. 读取像素 ---
        glm::vec2 _last_pos = mainInteractor.getMouseLastPos();
        LOGI("Picking at screen position: x=%f, y=%f, flipped_y=%d, viewport: %d x %d", _last_pos.x, _last_pos.y, static_cast<int>(m_height - _last_pos.y - 1), m_width, m_height);
        if (_last_pos.x < 0 || _last_pos.x >= m_width || _last_pos.y < 0 || _last_pos.y >= m_height) {
            LOGI("WARNING: Touch position out of bounds");
            return 0;
        }

        LOGI( "g->id is :%d", g.id );

        GLuint temp_id = 0;
        glReadPixels(
            static_cast<int>(_last_pos.x), 
            static_cast<int>(m_height - _last_pos.y - 1),
            1, 1, 
            GL_RED_INTEGER, 
            GL_UNSIGNED_INT, 
            &temp_id
        );

        // ! instanceDataVectorPtr
        instanceDataVectorPtr = nullptr;

        // --- 5. 恢复之前保存的 OpenGL 状态 ---
        glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
        glUseProgram(last_program);
        glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
        if (last_blend_enabled) {
            glEnable(GL_BLEND);
        }
        if (last_depth_test) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
        
        return static_cast<int>(temp_id);
    }

    void getInstanceData( std::vector<InstanceData>* instanceData ) {
        // 使用引用成员变量的新构造函数初始化方式
        if ( instanceData != nullptr ) {
            // ! instanceDataVectorPtr
            // 在类析构之前 将 instanceDataVectorPtr 设置为 nullptr 或者直接在PerformPickingInstancing() 中设置
            instanceDataVectorPtr = instanceData;
        }
    }

    void updatePadGlobalInstanceOffsetArray( InstanceOffset* offset ) {
        memcpy(g.instanceOffsets, offset, 
                sizeof(InstanceOffset) * INSTANCES_COUNT);
    }

private:
    int m_width, m_height;
    Globals& g;
    
    const Model& mainModel;
    const Camera& mainCamera;
    const CameraInteractor& mainInteractor;
    std::vector<InstanceData>* instanceDataVectorPtr;
    std::unique_ptr<IntFBO> mainFBO;
    std::unique_ptr<SilhouettesClass> mainProgram;
};