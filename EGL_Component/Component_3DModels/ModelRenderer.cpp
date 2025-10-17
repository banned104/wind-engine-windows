#include "ModelRenderer.hpp"
#include "macros.h" 

auto startTime = std::chrono::high_resolution_clock::now();


#ifdef __ANDROID__
ModelRenderer::ModelRenderer( ANativeWindow* window, const std::string& modelDir, int width, int height)
    : mWindow(window), mWidth(width), mHeight(height) {
#else
ModelRenderer::ModelRenderer(GLFWwindow* window, const std::string& modelDir, int width, int height)
    : mWindow(window), mWidth(width), mHeight(height) {
#endif

    // 移除重复的GLAD初始化，因为main.cpp已经初始化过了
    // 只需确保上下文是当前即可
    #ifndef __ANDROID__
    glfwMakeContextCurrent(mWindow);
    #else

    if (initEGL()) {
        LOGI( "Now is Android running" );
    } else {
        LOGE("Failed to initialize EGL");
        mIsInitialized = false;
        return;
    }
    
    #endif
    
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

    #ifdef __ANDROID__
    destroyEGL();
    #else
    destroyOpenGL();
    #endif
    // unique_ptr 会自动释放 mModel 和 mProgram
}

bool ModelRenderer::initOpenGL() {
    #ifndef __ANDROID__
    glfwMakeContextCurrent(mWindow);
    #else

    #endif
    
    
    // 验证OpenGL是否可用
    if (!glGetString(GL_VERSION)) {
        LOGE("OpenGL context not available");
        return false;
    }
    
    LOGI("OpenGL context verified successfully.");
    return true;
}

void ModelRenderer::draw() {
    // ========== 早期返回检查 ==========
    if (!mIsInitialized || !mOffscreenRenderer) {
        return;
    }
    
    #ifndef __ANDROID__
    // 显示加载界面（模型未加载完成时）
    if (!mIsModelLoaded) {
        drawLoadingView();
        return;
    }
    #endif

    // ========== 一次性初始化 ==========
    performFirstTimeInitialization();
    updateCameraIfNeeded();
    initializeTouchPadIfNeeded();
    
    // ========== 每帧计算和更新 ==========
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = mCamera->getViewMatrix();
    
    // 执行拾取操作（如果需要）
    performPickingIfRequested(modelMatrix, viewMatrix);

    // ========== 主渲染流程 ==========
    mOffscreenRenderer->beginFrame();
    
    if (mIsModelLoaded && mModel) {
        renderScene(viewMatrix, modelMatrix);
    } else {
        LOGE("mModel is NOT set");
    }

    mOffscreenRenderer->endFrame();
    mOffscreenRenderer->drawToScreen();
    #ifdef __ANDROID__
    eglSwapBuffers(mDisplay, mSurface);
    #else
    glfwSwapBuffers(mWindow);
    #endif
}

#ifdef __ANDROID__      /* 如果是编译为安卓.so 修改destroy实现 添加 initEGL */
bool ModelRenderer::initEGL() {
    // --- 这部分代码基本可以从你的 jni_cpp.cpp 中复制 ---
    mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (mDisplay == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed");
        return false;
    }

    if (eglInitialize(mDisplay, nullptr, nullptr) != EGL_TRUE) {
        LOGE("eglInitialize failed");
        return false;
    }

    EGLConfig config;
    EGLint numConfigs;
    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 24, // !! 添加深度缓冲配置，对于3D渲染至关重要 !!
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, // 使用 GLES 3
        EGL_NONE
    };

    if (eglChooseConfig(mDisplay, configAttribs, &config, 1, &numConfigs) != EGL_TRUE) {
        LOGE("eglChooseConfig failed");
        return false;
    }

    mSurface = eglCreateWindowSurface(mDisplay, config, mWindow, nullptr);
    if (mSurface == EGL_NO_SURFACE) {
        LOGE("eglCreateWindowSurface failed");
        return false;
    }

    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    mContext = eglCreateContext(mDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    if (mContext == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed");
        return false;
    }

    if (eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) != EGL_TRUE) {
        LOGE("eglMakeCurrent failed");
        return false;
    }
    LOGI("EGL Initialized Successfully.");
    return true;
}

void ModelRenderer::destroyEGL() {
    if (mDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (mContext != EGL_NO_CONTEXT) {
            eglDestroyContext(mDisplay, mContext);
        }
        if (mSurface != EGL_NO_SURFACE) {
            eglDestroySurface(mDisplay, mSurface);
        }
        eglTerminate(mDisplay);
    }
    mDisplay = EGL_NO_DISPLAY;
    mContext = EGL_NO_CONTEXT;
    mSurface = EGL_NO_SURFACE;
    ANativeWindow_release(mWindow);
}
#else /* 如果是编译为安卓.so 修改destroy实现 添加 initEGL */

void ModelRenderer::destroyOpenGL() {
    // 清理GLFW窗口
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}
#endif /* __ANDROID__ */

/* initGLES 在编译为.so时需要保留  */
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

    #ifndef __ANDROID__
    // 模型未完全载入时显示的OpenGL绘制的画面
    mLoadingViewProgram = std::make_unique<LoadingViewClass>();
    #endif
    
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

// ========== 私有辅助函数实现 ==========

void ModelRenderer::drawLoadingView() {
    LOGI("Renderer not initialized, Loading view is presenting.");
    mOffscreenRenderer->beginFrame();
    mLoadingViewProgram->use();
    mLoadingViewProgram->draw();
    mOffscreenRenderer->endFrame();
    mOffscreenRenderer->drawToScreen();
    #ifdef __ANDROID__
    eglSwapBuffers(mDisplay, mSurface);
    #else
    glfwSwapBuffers(mWindow);
    #endif
}

void ModelRenderer::performFirstTimeInitialization() {
    if (!mIsFirstDrawAfterModelLoaded) return;
    
    mIsFirstDrawAfterModelLoaded = false;
    LOGI("std::chrono::high_resolution_clock::now(); mIsFirstDrawAfterModelLoaded, used:%lld ms", 
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count());

    // 将模型数据从RAM上传到GPU
    mModel->uploadToGPU();

    // 初始化相机系统
    initializeCameraSystem();
    
    // 初始化渲染组件
    initializeRenderingComponents();
    
    // 初始化实例化数据
    initializeInstancedData();
    
    // 初始化纹理管理器
    initializeTextureManager();
}

void ModelRenderer::initializeCameraSystem() {
    mCamera = std::make_unique<Camera>();
    m_cameraInteractor = std::make_unique<CameraInteractor>(mCamera.get());
    
    // 计算模型尺寸和设置相机
    m_modelCenter = (mModel->boundsMin() + mModel->boundsMax()) * 0.5f;
    m_modelDepth = glm::length(mModel->boundsMax() - mModel->boundsMin());
    mCamera->setTarget(glm::vec3(0.0, 0.0, 0.0));
    mCamera->setDistance(m_modelDepth * 0.7f);
    std::cout << "Distance:" << m_modelDepth << std::endl;
}

void ModelRenderer::initializeRenderingComponents() {
    // 创建Shader程序
    mProgram = std::make_unique<ModelProgram>();
    glEnable(GL_DEPTH_TEST);
    LOGI("GLES Initialized for model rendering.");
    
    // 缓存Uniform位置
    mProgram->cacheUniformLocations();
    
    // 初始化UBO数据
    initializeUBOData();
    
    // 设置投影矩阵
    float aspect = static_cast<float>(mWidth) / static_cast<float>(mHeight);
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, m_modelDepth * 20.0f);
    
    // 初始化坐标轴渲染器
    AxisRenderer::Config axisConfig;
    axisConfig.length = m_modelDepth * 0.1f;
    axisConfig.depthTest = false;
    mAxis = std::make_unique<AxisRenderer>(axisConfig);
    
    // 初始化包围盒渲染器
    mBoundingBoxRenderer = std::make_unique<BoundingBoxRenderer>();
    if (!mBoundingBoxRenderer->initialize()) {
        LOGE("Failed to initialize BoundingBoxRenderer");
        mBoundingBoxRenderer.reset();
    } else {
        LOGI("BoundingBoxRenderer initialized successfully");
    }
}

void ModelRenderer::initializeUBOData() {
    // 零初始化所有字段
    m_ubo = {};
    m_ubo.proj = glm::mat4(1.0f);
    m_ubo.view = glm::mat4(1.0f);
    m_ubo.model = glm::mat4(1.0f);
    m_ubo.time = 0.0f;
    m_ubo.waveAmp = 1.0f;
    m_ubo.waveSpeed = 5.0f;
    m_ubo.pickedInstanceID = -1;  // 使用-1表示没有选中任何实例
    m_ubo.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_ubo.deltaX = 0.0f;
    m_ubo.deltaY = 0.0f;
    
    // 初始化实例偏移数组
    for (int i = 0; i < INSTANCES_COUNT; i++) {
        m_ubo.instanceOffsets[i] = m_instanceOffsets[i];
    }
    
    // 立即更新到GPU
    mProgram->updateGlobals(m_ubo);
}

void ModelRenderer::initializeInstancedData() {
    constexpr int instance_count = INSTANCES_COUNT;
    render_instance_data.resize(instance_count);
    generateInstanceData(render_instance_data, instance_count);
    mModel->setupInstances(render_instance_data);
    LOGI("Instances data generated.");
}

void ModelRenderer::initializeTextureManager() {
    m_textureManager = &GlobalTextureManager::getInstance();
    m_textureManager->initialize();
    
    // 加载图片到 map容器 -> m_textures; key = "cpp_vertexMovementTexture"
    bool flag_texture = m_textureManager->loadTexture(m_modelDir + "/chufengmask.jpg", "fadeEdgeMask", false);
    if (flag_texture) {
        LOGI(" (fadeEdgeMask) Load texture success");
    } else {
        LOGE("(fadeEdgeMask)Load texture failed");
    }
    
    //  BUGFIXED -> 检测到错误 m_texture 绑定纹理到了 TEXTURE0 与ModelLoader纹理冲突
    flag_texture = m_textureManager->bindToShader("fadeEdgeMask", mProgram->getProgramId(), "fadeEdgeMaskTexture");
    if (flag_texture) {
        LOGI("(fadeEdgeMask) Load texture success");
    } else {
        LOGE("(fadeEdgeMask) Load texture failed");
    }
}

void ModelRenderer::updateCameraIfNeeded() {
    if (!mCamera) return;
    
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
    lastTime = currentTime;
    mCamera->update(deltaTime);
}

void ModelRenderer::initializeTouchPadIfNeeded() {
    if (!mIsFirstTouchPadLoaded) return;
    
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
    }
    
    // 设置相机移动回调
    m_cameraInteractor->setOnMoveCallbackFunction([&](float deltaX, float deltaY) {
        if (deltaX != 0.0f || deltaY != 0.0f) {
            if (m_lastPickedID > 0 && m_lastPickedID <= INSTANCES_COUNT) {
                int instanceIndex = m_lastPickedID - 1;
                m_instanceOffsets[instanceIndex].deltaX += deltaX * 0.01f;
                m_instanceOffsets[instanceIndex].deltaY += deltaY * 0.01f;

                for (int i = 0; i < INSTANCES_COUNT; i++) {
                    m_ubo.instanceOffsets[i] = m_instanceOffsets[i];
                }
                m_touchPad->updatePadGlobalInstanceOffsetArray(m_instanceOffsets);
            }
        }
    });
}

void ModelRenderer::performPickingIfRequested(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix) {
    if (!((m_touchPad && m_pickRequested) || mIsFirstAutomaticPicking)) return;
    
    m_pickRequested = false;
    mIsFirstAutomaticPicking = false;

    m_globals->modelMatrix = modelMatrix;
    m_globals->viewMatrix = viewMatrix;
    m_globals->projMatrix = mCamera->getProjectionMatrix();

    #ifdef ENABLE_INSTANCING
    int pickedID = m_touchPad->performPickingInstancing();
    #else 
    int pickedID = m_touchPad->performPicking();
    #endif

    m_lastPickedID = pickedID;
    m_cameraInteractor->mPickedID = pickedID;

    if (pickedID == BACKGROUND_ID) {
        LOGI("Picked background (no model at position) %d", pickedID);
    } else if (pickedID > 0) {
        LOGI("Picked model with ID: %d", pickedID);
    } else {
        LOGI("Picked nothing pickID: %d", pickedID);
    }
}

void ModelRenderer::renderScene(glm::mat4& viewMatrix, const glm::mat4& modelMatrix) {
    // 渲染天空盒
    //renderSkybox(viewMatrix);
    
    // 设置模型渲染状态
    setupModelRenderingState();
    
    // 更新UBO数据
    updateUBOData(viewMatrix, modelMatrix);
    
    // 渲染模型
    renderModel();
    
    // 渲染辅助元素
    renderAuxiliaryElements(viewMatrix, modelMatrix);
}

void ModelRenderer::renderSkybox(glm::mat4& viewMatrix) {
    GLint depthFunc;
    GLboolean depthMask;
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    
    glm::mat4 originalView = viewMatrix;
    mSkybox->Draw(viewMatrix, m_projectionMatrix);
    glDepthMask(depthMask);
    glDepthFunc(depthFunc);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    viewMatrix = originalView;
}

void ModelRenderer::setupModelRenderingState() {
    static int frameCount = 0;
    frameCount++;
    
    mProgram->use();
    m_textureManager->activateTextures();
}

void ModelRenderer::updateUBOData(const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) {
    static glm::mat4 lastProj, lastView, lastModel;
    static float lastUboTime = -1.0f;
    static auto startTime = std::chrono::high_resolution_clock::now();
    
    bool needUpdate = false;
    
    glm::mat4 currentProj = mCamera->getProjectionMatrix();
    glm::mat4 currentView = mCamera->getViewMatrix();

    // 检查矩阵变化
    if (lastProj != currentProj || lastView != currentView || lastModel != modelMatrix) {
        needUpdate = true;
        lastProj = currentProj;
        lastView = currentView;
        lastModel = modelMatrix;
    }

    m_ubo.proj = currentProj;
    m_ubo.view = currentView;
    m_ubo.model = modelMatrix;

    // 更新动画时间
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    float currentTime = elapsed.count() / 1000.0f;
    float wrappedTime = fmod(currentTime, 10.0f);

    if (lastUboTime != wrappedTime) {
        needUpdate = true;
        lastUboTime = wrappedTime;
    }

    m_ubo.time = wrappedTime;
    m_ubo.pickedInstanceID = m_lastPickedID;
    
    // 更新模型边界
    if (mModel) {
        m_ubo.boundMax = mModel->boundsMax();
        m_ubo.boundMin = mModel->boundsMin();
        mProgram->updateGlobals(m_ubo);
    }

    if (needUpdate) {
        mProgram->updateGlobals(m_ubo);
    }
}

void ModelRenderer::renderModel() {
    #ifndef ENABLE_INSTANCING
    mModel->Draw(mProgram->getProgramId());
    #else 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    mModel->DrawInstancedWind(mProgram->getProgramId(), INSTANCES_COUNT);
    glDepthMask(GL_TRUE);
    #endif
}

void ModelRenderer::renderAuxiliaryElements(const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) {
    // 渲染坐标轴
    glDisable(GL_DEPTH_TEST);
    mAxis->render(viewMatrix, m_projectionMatrix);
    glEnable(GL_DEPTH_TEST);

    // 渲染包围盒
    if (mShowBoundingBox && mBoundingBoxRenderer && mModel) {
        renderBoundingBoxes(viewMatrix, modelMatrix);
    }
}

void ModelRenderer::renderBoundingBoxes(const glm::mat4& viewMatrix, const glm::mat4& modelMatrix) {
    glm::vec3 minBounds = mModel->boundsMin();
    glm::vec3 maxBounds = mModel->boundsMax();
    glm::vec3 boundingBoxColor(1.0f, 1.0f, 0.0f);
    
    // 渲染全局模型包围盒
    glm::mat4 mvpMatrix = mCamera->getProjectionMatrix() * viewMatrix * modelMatrix;
    mBoundingBoxRenderer->drawBoundingBox(minBounds, maxBounds, mvpMatrix, boundingBoxColor);
    
    #ifdef ENABLE_INSTANCING
    // 渲染实例包围盒
    glm::vec3 instanceColor(0.0f, 1.0f, 1.0f);
    for (int i = 0; i < INSTANCES_COUNT; i++) {
        glm::mat4 instanceModelMatrix = render_instance_data[i].modelMatrix;
        glm::mat4 instanceMvpMatrix = mCamera->getProjectionMatrix() * viewMatrix * instanceModelMatrix;
        mBoundingBoxRenderer->drawBoundingBox(minBounds, maxBounds, instanceMvpMatrix, instanceColor);
    }
    #endif
}

