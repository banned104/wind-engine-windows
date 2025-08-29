#pragma once

#define __OFFSCREEN_RENDERING__
#define __3D_CAMERA__
#define __UNIVERSAL_3D_MODEL__

// 模型编号 之后会拓展加载多个实例
#define RENDER_GLOBAL_MODEL_INSTANCE_ID 3


#include <string>
#include <memory>
#include <vector>
#include <thread> // 用于异步加载模型的线程
#include <atomic>
#include <ctime>
#include <random>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ModelLoader_Universal_Instancing.hpp"
// #include "Component_Shader_Blinn_Phong/PhongModelProgram.hpp"
#include "Component_Shader_Blinn_Phong/WindShader.hpp"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "Camera.hpp"
#include "CameraInteractor.hpp"
#include "OffscreenRenderer.hpp"
#include "BoundingBoxRenderer.hpp"
#include "CommonTypes.hpp"

#include "SkyBox.hpp"
#include "axis_helper.hpp"

#ifdef ENABLE_INSTANCING
#include "FlexableTouchPad_Instancing.hpp"
#else
#include "FlexableTouchPad.hpp"
#endif

#include "Component_TextureManager/TextureManager.hpp"

struct Globals;




class ModelRenderer
{
public:
    // 构造函数，接收GLFW窗口、模型路径和视口尺寸
    ModelRenderer(GLFWwindow* window, const std::string &modelDir, int width, int height);

    // 析构函数，用于释放资源
    ~ModelRenderer();
    // 主绘制函数
    void draw();

    // 获取相机对象的引用，以便从外部控制
    Camera &getCamera();
    CameraInteractor* getInteractor() { return m_cameraInteractor.get(); }

    // 生成实例化数据
    void generateInstanceData( std::vector<InstanceData>& instanceData, int instanceCount );

    // 包围盒控制方法
    void setBoundingBoxVisible(bool visible) { mShowBoundingBox = visible; }
    bool isBoundingBoxVisible() const { return mShowBoundingBox; }
    void requestPick() { m_pickRequested = true; }

private:
    bool mIsInitialized = false;
    // 初始化 OpenGL 环境
    bool initOpenGL();
    // 初始化 OpenGL 相关内容（模型、着色器、矩阵）
    void initGLES(const std::string &modelDir);
    // 释放 OpenGL 资源
    void destroyOpenGL();

    GLFWwindow* mWindow;
    int mWidth;
    int mHeight;

    // 模型异步加载相关
    std::thread mLoadingThread;
    std::atomic<bool> mIsModelLoaded{false}; // 原子化布尔值 保证线程安全
    std::atomic<bool> mIsFirstDrawAfterModelLoaded{true};
    std::atomic<bool> mIsFirstTouchPadLoaded{true};

    // 渲染相关
    std::unique_ptr<Model> mModel;
    std::unique_ptr<ModelProgram> mProgram;
    std::unique_ptr<LoadingViewClass> mLoadingViewProgram;
    std::unique_ptr<OffscreenRenderer> mOffscreenRenderer; // 使用组合

    
    // 包围盒渲染器
    std::unique_ptr<BoundingBoxRenderer> mBoundingBoxRenderer;
    bool mShowBoundingBox = true; // 控制是否显示包围盒

    // 天空盒
    std::unique_ptr<Skybox> mSkybox;

    // 坐标轴
    std::unique_ptr<AxisRenderer> mAxis;

    
    std::unique_ptr<Camera> mCamera;
    std::unique_ptr<CameraInteractor> m_cameraInteractor;
    std::unique_ptr<FlexableTouchPadClass> m_touchPad;
    std::unique_ptr<Globals> m_globals;
    // 获取相机对象的引用，以便从外部控制
    // void on_touch_down(float x, float y) {
    //     // 假设左键按下
    //     m_cameraInteractor->onMouseDown(x, y, CameraInteractor::MouseButton::Left);
    // }

    void on_touch_down(float x, float y) {
        // 先执行拾取操作
        m_pickRequested = true;     // 会让渲染循环中的处理逻辑执行 if (m_touchPad && m_pickRequested) 
        // 保存触摸位置，在下一帧的draw()中处理拾取结果
        m_cameraInteractor->onMouseDown(x, y, CameraInteractor::MouseButton::Left);
    }

    // void on_touch_move(float x, float y) {
    //     m_cameraInteractor->onMouseMove(x, y);
    // }

    void on_touch_move(float x, float y) {
        // 如果触摸的是背景，则控制相机移动
        if (m_lastPickedID == -1) {
            m_cameraInteractor->onMouseMove(x, y);
        } else {
            // 如果触摸的是模型，则修改UBO参数实现拖拽功能
            // 计算触摸移动的距离
            glm::vec2 currentPos(x, y);
            glm::vec2 lastPos = m_cameraInteractor->getMouseLastPos();
            glm::vec2 delta = currentPos - lastPos;
            
            // 更新UBO中的delta值，用于Shader中的实例偏移
            m_ubo.deltaX = delta.x;
            m_ubo.deltaY = delta.y;
            mProgram->updateGlobalsPartDelta(delta.x, delta.y);
            
            // 更新最后的触摸位置
            m_cameraInteractor->onMouseMove(x, y);
        }
    }


    void on_touch_up(float x, float y) {
        m_cameraInteractor->onMouseUp();

        // 重置拾取ID
        m_lastPickedID = -1;
    }

    void on_scroll(float delta) {
        m_cameraInteractor->onMouseScroll(delta);
    }


    glm::mat4 m_projectionMatrix;
    glm::vec3 m_modelCenter;
    float m_modelDepth;
    std::atomic<bool> m_pickRequested{false};

    // instancing
    std::vector<InstanceData> render_instance_data;

    // 触摸相关
    int m_lastPickedID = -1;        // 跟踪最后一次拾取的模型
    ModelProgram::WindUBO m_ubo;    // 存储当前UBO参数

    // 每个实例的独立偏移状态
    InstanceOffset m_instanceOffsets[INSTANCES_COUNT];

    // 额外纹理加载 使用全局纹理管理器
    std::string m_modelDir = "";
    // auto& m_textureManager = GlobalTextureManager::getInstance();
    GlobalTextureManager* m_textureManager = nullptr;

};