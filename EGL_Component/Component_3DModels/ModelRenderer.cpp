#include "ModelRenderer.hpp"
#include "macros.h" 

auto startTime = std::chrono::high_resolution_clock::now();

ModelRenderer::ModelRenderer(GLFWwindow* window, const std::string& modelDir, int width, int height)
    : mWindow(window), mWidth(width), mHeight(height) {
    // 移除重复的GLAD初始化，因为main.cpp已经初始化过了
    // 只需确保上下文是当前即可
    glfwMakeContextCurrent(mWindow);
    
    // 验证OpenGL是否已初始化
    if (!glGetString(GL_VERSION)) {
        LOGE("OpenGL context not available");
        mIsInitialized = false;
        return;
    }
    
    // 初始化所有实例的偏移为0
    for (int i = 0; i < INSTANCES_COUNT; i++) {
        m_instanceOffsets[i].deltaX = 0.0f;
        m_instanceOffsets[i].deltaY = 0.0f;
        m_instanceOffsets[i]._padding1 = 0.0f;
        m_instanceOffsets[i]._padding2 = 0.0f;
    }

    // 确保OffscreenRenderer总是被初始化
    try {
        mOffscreenRenderer = std::make_unique<OffscreenRenderer>(width, height);
        mIsInitialized = true;
    } catch (const std::exception& e) {
        LOGE("Failed to create OffscreenRenderer: %s", e.what());
        mIsInitialized = false;
    }

    if (mIsInitialized) {
        initGLES(modelDir);
    }
}

ModelRenderer::~ModelRenderer() {
    // 渲染器销毁之前 确保线程已经执行完毕
    if ( mLoadingThread.joinable() ) {
        mLoadingThread.join();
    }
    
    // 清理包围盒渲染器资源
    if (mBoundingBoxRenderer) {
        mBoundingBoxRenderer->cleanup();
        mBoundingBoxRenderer.reset();
    }

    destroyOpenGL();
    // unique_ptr 会自动释放 mModel 和 mProgram

}

bool ModelRenderer::initOpenGL() {
    // 只需确保上下文是当前即可，不需要重复初始化GLAD
    glfwMakeContextCurrent(mWindow);
    
    // 验证OpenGL是否可用
    if (!glGetString(GL_VERSION)) {
        LOGE("OpenGL context not available");
        return false;
    }
    
    LOGI("OpenGL context verified successfully.");
    return true;
}

void ModelRenderer::initGLES(const std::string& modelDir) {
    m_modelDir = modelDir;
    // 1. 加载模型
    startTime = std::chrono::high_resolution_clock::now();
    try {
        std::string modelPath = modelDir + "/chufeng.obj";              // 37ms
        // std::string modelPath = modelDir + "/fox.gltf";
        // std::string modelPath = modelDir + "/ddm.glb";              // 204ms
        // std::string modelPath = modelDir + "/r35/r35.fbx";
        // std::string modelPath = modelDir + "/lk1a.gltf";
        LOGI("Loading model from: %s", modelPath.c_str());
        std::uintmax_t fileSize = 0;
        try {
            fileSize = std::filesystem::file_size( modelPath );
            LOGI("Filesize is %lu", fileSize);
        } catch(const std::exception& e) {
            LOGE( "Get Filesize failed" );
        }
        
        // 模型小于1MB就不需要多线程加载(甚至10MB); Bytes为单位; wind.obj 753KB
        if ( fileSize/1000 > 1000 ) {
            // 使用多线程加载模型
            mLoadingThread = std::thread( [this, modelPath](){
                try {
                auto loadedModel = std::make_unique<Model>( modelPath );
                mModel = std::move( loadedModel );
                
                mIsModelLoaded = true;
                LOGI( "Model loading finished successfully on background thread." );
                } catch ( const std::exception& e ) {
                    LOGE( "Thread Fail: %s", e.what() );
                }
            } );
        } else {
            try {
                auto loadedModel = std::make_unique<Model>( modelPath );
                mModel = std::move( loadedModel );
                mIsModelLoaded = true;
                LOGI( "Direct load model successfully" );
            } catch ( const std::exception& e ) {
                LOGE( "Direct load model failed: %s", e.what() );
            }
        }

    } catch (const std::exception& e) {
        LOGE("Failed to load model: %s", e.what());
        return;
    }
    // 模型未完全载入时显示的OpenGL绘制的画面
    mLoadingViewProgram = std::make_unique<LoadingViewClass>();
     mSkybox = std::make_unique<Skybox>(modelDir);

    // 开启混合 透明度
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask( GL_TRUE );

    
    // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    LOGI("std::chrono::high_resolution_clock::now(); initGLES finished, used:%lld ms", 
            std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - startTime ).count()
            );
}

void ModelRenderer::draw() {
    if (!mIsInitialized || !mOffscreenRenderer) {
        // 显示加载界面或错误信息
        return;
    }
    
    // * 若模型太大会出现白屏问题, 此处用于显示加载界面或者什么都不做( 仅显示透明 )
    if (!mIsModelLoaded) { 
        LOGI("Renderer not initialized, Loading view is presenting.");
        mOffscreenRenderer->beginFrame(); // 准备FBO
        // glClearColor( 0.1f, 0.2f, 0.3f, 1.0f );

        mLoadingViewProgram->use();
        // mLoadingViewProgram->updataGlobals()
        mLoadingViewProgram->draw();

        mOffscreenRenderer->endFrame();   // 解析FBO
        mOffscreenRenderer->drawToScreen(); // 将结果绘制到屏幕
        glfwSwapBuffers(mWindow);
        return;
    } 

    if ( mIsFirstDrawAfterModelLoaded ) {
        mIsFirstDrawAfterModelLoaded = false;

        LOGI("std::chrono::high_resolution_clock::now(); mIsFirstDrawAfterModelLoaded, used:%lld ms", 
        std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - startTime ).count()
        );

        // ! 在主线程中将数据从RAM搬到GPU RAM  子线程没有 OpenGL Context
        mModel->uploadToGPU();

        mCamera = std::make_unique<Camera>();
        // 传递相机指针给CameraInterator
        m_cameraInteractor = std::make_unique<CameraInteractor>(mCamera.get());

        m_modelCenter = ( mModel->boundsMin() + mModel->boundsMax() ) *0.5f;
        m_modelDepth = glm::length( mModel->boundsMax() - mModel->boundsMin() );
        mCamera->setTarget( glm::vec3( 0.0, 0.0, 0.0 ) );
        mCamera->setDistance( m_modelDepth * 0.7f );      // 根据模型大小设置一个合适的初始距离

        // PhongModelProgram.hpp 中类的名字也是ModelProgram 要切换Shader 只需要改变包含的 .hpp 文件即可
        mProgram = std::make_unique<ModelProgram>();

        glEnable(GL_DEPTH_TEST);
        LOGI("GLES Initialized for model rendering.");

        // 在 glProgramLink 方法执行之后运行获取 Shader中变量位置并保存至缓存中的方法
        mProgram->cacheUniformLocations();

        // 投影矩阵
        float aspect = static_cast<float>(mWidth) / static_cast<float>(mHeight);
        m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, m_modelDepth * 20.0f);
        
        // 4. 初始化包围盒渲染器
        mBoundingBoxRenderer = std::make_unique<BoundingBoxRenderer>();
        if (!mBoundingBoxRenderer->initialize()) {
            LOGE("Failed to initialize BoundingBoxRenderer");
            mBoundingBoxRenderer.reset();
        } else {
            LOGI("BoundingBoxRenderer initialized successfully");
        }

        // 实例化初始化
        constexpr int instance_count = INSTANCES_COUNT;
        // std::vector<InstanceData> render_instance_data( instance_count );
        render_instance_data.resize( instance_count );
        generateInstanceData( render_instance_data, instance_count );
        mModel->setupInstances( render_instance_data );
        LOGI("Instances data generated.");

        
        // 初始化额外纹理管理器
        m_textureManager = &GlobalTextureManager::getInstance();
        m_textureManager->initialize();
        bool _flag_texture = false;
        _flag_texture = m_textureManager->loadTexture( m_modelDir + "/headtailmask.jpg", "cpp_vertexMovementTexture", false );  // unordered_map 在这个单例中创建 cpp_vertexMovementTexture 这个id 与 这个.jpg文件的唯一关联
        if (_flag_texture) { LOGI("Load texture success"); }
        else { LOGE("Load texture failed"); }

        _flag_texture = m_textureManager->bindToShader( "cpp_vertexMovementTexture", mProgram->getProgramId(), "vertexMovementTexture" );   // 绑定纹理到Shader // 绑定之后还需要在循环中激活
        if (_flag_texture) { LOGI("Load texture success"); }
        else { LOGE("Load texture failed"); }
        // [ERROR][GlobalTextureManager] Uniform not found in shader : vertexMovementTexture
    }

    if ( mCamera ) {
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;
        mCamera->update(deltaTime);
    }

    // ! TouchPad 初始化
    if ( mIsFirstTouchPadLoaded ) {
        mIsFirstTouchPadLoaded = false;
        try {
            m_globals = std::make_unique<Globals>();
            m_globals->id = RENDER_GLOBAL_MODEL_INSTANCE_ID;
            m_touchPad = std::make_unique<FlexableTouchPadClass>(
                mWidth, mHeight,
                *m_globals,
                *mModel,
                *mCamera,
                *m_cameraInteractor
            );
        } catch (const std::runtime_error& e) {
            LOGE("Error creating FlexableTouchPad: %s", e.what());
            // 在这里可以添 加更多的错误处理逻辑，比如禁用触摸功能
        }
        m_cameraInteractor->setOnMoveCallbackFunction( [&](float deltaX, float deltaY) {
            // 如果deltaX和deltaY都为0，说明是重置操作，但我们不再重置
            // 现在每个实例保持独立的偏移状态
            if (deltaX != 0.0f || deltaY != 0.0f) {
                // 只更新当前选中实例的偏移
                if (m_lastPickedID > 0 && m_lastPickedID <= INSTANCES_COUNT) {
                    int instanceIndex = m_lastPickedID - 1;  // 实例ID从1开始，数组索引从0开始
                    m_instanceOffsets[instanceIndex].deltaX += deltaX * 0.01f;
                    m_instanceOffsets[instanceIndex].deltaY += deltaY * 0.01f;

                    // 更新UBO中的实例偏移数组
                    for (int i = 0; i < INSTANCES_COUNT; i++) {
                        m_ubo.instanceOffsets[i] = m_instanceOffsets[i];
                    }
                    // 更新FlexablePad的位置和形变
                    m_touchPad->updatePadGlobalInstanceOffsetArray( m_instanceOffsets );
                }
            }
        } );
    }
    
    // 1. 计算 MVP 矩阵
    glm::mat4 modelMatrix = glm::mat4(1);
    // 视图矩阵
    glm::mat4 viewMatrix = mCamera->getViewMatrix();

    if (m_touchPad && m_pickRequested) {
        m_pickRequested = false;

        m_globals->modelMatrix = modelMatrix;
        m_globals->viewMatrix = viewMatrix;
        m_globals->projMatrix = mCamera->getProjectionMatrix();

        #ifdef ENABLE_INSTANCING        // 这个宏在CMake中定义
        // 在之前的初始化中已经传递 InstanceData 到mModel中, 这里touchpad 又获得了 mModel , 所以不需要再传递这个数据
        // m_touchPad->getInstanceData( &render_instance_data );
        int pickedID = m_touchPad->performPickingInstancing();
        #else 
        int pickedID = m_touchPad->performPicking();
        #endif

        // 保存拾取结果
        m_lastPickedID = pickedID;
        m_cameraInteractor->mPickedID = pickedID;

        if (pickedID == -1) {
            LOGI("Picked background (no model at position)");
        } else if (pickedID > 0) {
            LOGI("Picked model with ID: %d", pickedID);
            // 不再在这里设置deltaX和deltaY，这些值现在由回调函数控制
        } else {
            LOGI("Picked nothing pickID: %d", pickedID);
        }
        
    }


    mOffscreenRenderer->beginFrame(); // 准备FBO
    // 绘制3D场景 + 当后台线程加载完模型之后再绘制
    if ( mIsModelLoaded && mModel ) {
        // 绘制天空盒
        mSkybox->Draw( viewMatrix, m_projectionMatrix );

        // 帧数统计
        static int frameCount = 0;
        frameCount++;
        mProgram->use();

        // 激活纹理单元
        m_textureManager->activateTextures();

        // todo 现在每一帧都更新MVP矩阵, 后续考虑优化
        // 更新Shader中的 MVP 矩阵
        // 优化：只在必要时更新UBO，减少GPU数据传输
        static glm::mat4 lastProj, lastView, lastModel;
        static float lastUboTime = -1.0f;

        bool needUpdate = false;

        glm::mat4 currentProj = mCamera->getProjectionMatrix();
        glm::mat4 currentView = mCamera->getViewMatrix();

        // 检查矩阵是否发生变化
        if (lastProj != currentProj || lastView != currentView || lastModel != modelMatrix) {
            needUpdate = true;
            lastProj = currentProj;
            lastView = currentView;
            lastModel = modelMatrix;
        }

        m_ubo.proj = currentProj;
        m_ubo.view = currentView;
        m_ubo.model = modelMatrix;

        // 动画参数 - 修复：确保时间连续平滑传递
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);

        // 使用连续的时间值，确保流动效果平滑
        float currentTime = elapsed.count() / 1000.0f;  // 转换为秒
        float wrappedTime = fmod(currentTime, 10.0f);   // 每10秒循环一次

        // 检查时间是否发生变化（用于优化UBO更新）
        if (lastUboTime != wrappedTime) {
            needUpdate = true;
            lastUboTime = wrappedTime;
        }

        m_ubo.time = wrappedTime;
        m_ubo.pickedInstanceID = m_lastPickedID;
        
        // 如果没有初始化过UBO参数, 则设置默认值, 初始参数设置
        static bool uboInitialized = false;
        if (!uboInitialized) {
            m_ubo.waveAmp = 1.0f;
            m_ubo.waveSpeed = 5.0f;
            needUpdate = true;
            uboInitialized = true;
        }
        
        if ( mModel ) {
            m_ubo.boundMax = mModel->boundsMax();
            m_ubo.boundMin = mModel-> boundsMin();
        }

        // 只在有变化时更新UBO
        if (needUpdate) {
            mProgram->updateGlobals(m_ubo);
        }


        // 调用模型的 Draw 函数 绘制模型 之前加载到GPU RAM中的纹理会被调用
        #ifndef ENABLE_INSTANCING
        mModel->Draw(mProgram->getProgramId());
        #else 
        // mModel->DrawInstanced( mProgram->getProgramId(), INSTANCES_COUNT );
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);      // 绘制模型前禁用深度写入, 但是保持深度测试
        mModel->DrawInstancedWind( mProgram->getProgramId(), INSTANCES_COUNT  );
        glDepthMask(GL_TRUE);       // 绘制结束恢复深度写入
        #endif


        // 绘制包围盒
        if (mShowBoundingBox && mBoundingBoxRenderer && mModel) {
            glm::vec3 minBounds = mModel->boundsMin();
            glm::vec3 maxBounds = mModel->boundsMax();
            glm::vec3 boundingBoxColor(1.0f, 1.0f, 0.0f); // 黄色线框
            
            // 绘制全局模型的包围盒
            glm::mat4 mvpMatrix = mCamera->getProjectionMatrix() * mCamera->getViewMatrix() * modelMatrix;
            mBoundingBoxRenderer->drawBoundingBox(minBounds, maxBounds, mvpMatrix, boundingBoxColor);
            
            // 为每个实例绘制包围盒
            #ifdef ENABLE_INSTANCING
            for (int i = 0; i < INSTANCES_COUNT; i++) {
                // 获取实例的模型矩阵
                glm::mat4 instanceModelMatrix = render_instance_data[i].modelMatrix;
                // 计算实例的MVP矩阵
                glm::mat4 instanceMvpMatrix = mCamera->getProjectionMatrix() * mCamera->getViewMatrix() * instanceModelMatrix;
                // 使用不同颜色区分不同实例的包围盒
                glm::vec3 instanceColor(0.0f, 1.0f, 1.0f); // 青色
                // 绘制实例的包围盒
                mBoundingBoxRenderer->drawBoundingBox(minBounds, maxBounds, instanceMvpMatrix, instanceColor);
            }
            #endif
        }


    } else {
        LOGE("mModel is NOT set");
    }

    mOffscreenRenderer->endFrame();   // 解析FBO
    mOffscreenRenderer->drawToScreen(); // 将结果绘制到屏幕

    // 交换缓冲区
    glfwSwapBuffers(mWindow);
}

void ModelRenderer::destroyOpenGL() {
    // 清理GLFW窗口
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}


Camera& ModelRenderer::getCamera() {
     return *mCamera; 
}



void  ModelRenderer::generateInstanceData( std::vector<InstanceData>& instanceData, int instanceCount ) {
    // std::vector<InstanceData_Adjust_Static> static_data = {
    //     { 0.1f, glm::vec3( -0.3f, 0.0f, 0.0f ) },  // 增大x轴间距
    //     { 0.1f, glm::vec3( -0.1f, 0.0f, 0.0f ) },
    //     { 0.1f, glm::vec3( 0.1f, 0.0f, 0.0f ) },
    //     { 0.1f, glm::vec3( 0.3f, 0.0f, 0.0f ) },
    // };
    float scale = INSTANCE_SCALE;
    float translate_factor = abs(mModel->scaled_boundsMin(scale).x - mModel->scaled_boundsMax(scale).x);
    std::vector<InstanceData_Adjust_Static> static_data = {
        { scale, glm::vec3( -1.5f * translate_factor, 0.0f, 0.0f ) },  // 左下
        { scale, glm::vec3( -0.5f * translate_factor, 0.0f, 0.0f ) },   // 左上
        { scale, glm::vec3( 0.5f * translate_factor, 0.0f, 0.0f ) },   // 右下
        { scale, glm::vec3( 1.5f * translate_factor, 0.0f, 0.0f )},    // 右上
    };

    // 根据静态数据设置实例
    for (int i = 0; i < instanceCount; i++) {
        const auto& data = static_data[i];
        // 创建模型矩阵
        glm::mat4 model = glm::mat4(1.0f);
        
        // 应用位置
        model = glm::translate(model, data.position);
        
        // 应用缩放
        model = glm::scale(model, glm::vec3(data.scale));
        
        // 设置实例数据
        instanceData[i].modelMatrix = model;
        // 设置实例ID
        instanceData[i].instanceId = i + 1;
    }
}