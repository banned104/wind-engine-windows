#include <iostream>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>

#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#else
// GLFW + GLAD
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif


// 项目组件
#include "EGL_Component/Component_3DModels/ModelRenderer.hpp"

// 全局变量
static std::atomic<bool> g_is_rendering{false};
static ModelRenderer* g_renderer = nullptr;
static GLFWwindow* g_window = nullptr;

// 触摸事件回调函数（桌面版本）
void onTouchDown(float x, float y) {
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMouseDown(x, y, CameraInteractor::MouseButton::Left);
        g_renderer->requestPick();
    }
}

void onTouchMove(float x, float y) {
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMouseMove(x, y);
    }
}

void onTouchUp(float x, float y) {
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMouseUp();
    }
}

void onScroll(float delta) {
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMouseScroll(delta);
    }
}

// GLFW回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    std::cout << "Window resized to: " << width << "x" << height << std::endl;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        if (action == GLFW_PRESS) {
            onTouchDown(static_cast<float>(xpos), static_cast<float>(ypos));
        } else if (action == GLFW_RELEASE) {
            onTouchUp(static_cast<float>(xpos), static_cast<float>(ypos));
        }
    }
    
    // 右键用于相机平移
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        if (action == GLFW_PRESS && g_renderer && g_renderer->getInteractor()) {
            g_renderer->getInteractor()->onMouseDown(static_cast<float>(xpos), static_cast<float>(ypos), 
                                                   CameraInteractor::MouseButton::Right);
        } else if (action == GLFW_RELEASE && g_renderer && g_renderer->getInteractor()) {
            g_renderer->getInteractor()->onMouseUp();
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool isDragging = false;
    
    // 左键或右键拖拽
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS || 
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (!isDragging) {
            isDragging = true;
        } else {
            onTouchMove(static_cast<float>(xpos), static_cast<float>(ypos));
        }
    } else {
        isDragging = false;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    onScroll(static_cast<float>(yoffset));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    // 切换包围盒显示
    if (key == GLFW_KEY_B && action == GLFW_PRESS && g_renderer) {
        bool currentState = g_renderer->isBoundingBoxVisible();
        g_renderer->setBoundingBoxVisible(!currentState);
        std::cout << "Bounding box " << (currentState ? "hidden" : "shown") << std::endl;
    }
    
    // 显示帮助信息
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        std::cout << "\n=== Wind Engine Controls ===" << std::endl;
        std::cout << "ESC - Exit application" << std::endl;
        std::cout << "H - Show this help" << std::endl;
        std::cout << "B - Toggle bounding box visibility" << std::endl;
        std::cout << "Left Mouse - Rotate camera / Select and move instances" << std::endl;
        std::cout << "Right Mouse - Pan camera" << std::endl;
        std::cout << "Mouse Wheel - Zoom in/out" << std::endl;
        std::cout << "===========================\n" << std::endl;
    }
}

// 停止渲染
void stopRender() {
    g_is_rendering = false;
    
    if (g_renderer) {
        delete g_renderer;
        g_renderer = nullptr;
    }
}

// 开始渲染
void startRender(const std::string& modelDir, int width, int height) {
    if (g_is_rendering) {
        stopRender();
    }

    g_is_rendering = true;
    
    try {
        g_renderer = new ModelRenderer(g_window, modelDir, width, height);
        std::cout << "ModelRenderer created successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create ModelRenderer: " << e.what() << std::endl;
        g_is_rendering = false;
    }
}

// 初始化GLFW和GLAD
bool initGLFW(int width = 1920, int height = 1080) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // 设置OpenGL版本和配置
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // 启用多重采样
    glfwWindowHint(GLFW_SAMPLES, 4);

    // 创建窗口
    g_window = glfwCreateWindow(width, height, "Wind Engine - Desktop", nullptr, nullptr);
    if (!g_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(g_window);
    
    // 启用垂直同步
    glfwSwapInterval(1);

    // 设置回调函数
    glfwSetFramebufferSizeCallback(g_window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(g_window, mouse_button_callback);
    glfwSetCursorPosCallback(g_window, cursor_pos_callback);
    glfwSetScrollCallback(g_window, scroll_callback);
    glfwSetKeyCallback(g_window, key_callback);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // 设置视口
    int fb_width, fb_height;
    glfwGetFramebufferSize(g_window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    // 启用多重采样
    glEnable(GL_MULTISAMPLE);

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return true;
}

// 清理资源
void cleanup() {
    stopRender();
    
    if (g_window) {
        glfwDestroyWindow(g_window);
        g_window = nullptr;
    }
    
    glfwTerminate();
}

int main(int argc, char *argv[]) {
    std::cout << "=== Wind Engine Desktop Version ===" << std::endl;
    std::cout << "Starting 3D Model Viewer..." << std::endl;

    // 解析命令行参数
    std::string modelDir = "models"; // 默认模型目录
    int width = 1920, height = 1080;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--model" && i + 1 < argc) {
            modelDir = argv[++i];
        } else if (arg == "--width" && i + 1 < argc) {
            width = std::stoi(argv[++i]);
        } else if (arg == "--height" && i + 1 < argc) {
            height = std::stoi(argv[++i]);
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --model <path>    Model directory path (default: models)" << std::endl;
            std::cout << "  --width <width>   Window width (default: 1920)" << std::endl;
            std::cout << "  --height <height> Window height (default: 1080)" << std::endl;
            std::cout << "  --help, -h        Show this help message" << std::endl;
            return 0;
        }
    }

    // 初始化GLFW和GLAD
    if (!initGLFW(width, height)) {
        std::cerr << "Failed to initialize GLFW/GLAD" << std::endl;
        return -1;
    }

    std::cout << "Model directory: " << modelDir << std::endl;
    std::cout << "Window size: " << width << "x" << height << std::endl;
    std::cout << "Press H for help, ESC to exit" << std::endl;

    // 获取实际窗口尺寸
    int fb_width, fb_height;
    glfwGetFramebufferSize(g_window, &fb_width, &fb_height);

    // 开始渲染
    startRender(modelDir, fb_width, fb_height);

    if (!g_is_rendering) {
        std::cerr << "Failed to start rendering" << std::endl;
        cleanup();
        return -1;
    }

    // 性能统计
    double lastTime = glfwGetTime();
    int frameCount = 0;
    double frameTime = 0.0;

    // 主循环 - 桌面版本单线程渲染
    while (!glfwWindowShouldClose(g_window) && g_is_rendering) {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // 处理窗口事件
        glfwPollEvents();
        
        // 渲染场景
        if (g_renderer) {
            g_renderer->draw();
        }
        
        // 性能统计（每秒输出一次）
        frameCount++;
        frameTime += deltaTime;
        if (frameTime >= 1.0) {
            double fps = frameCount / frameTime;
            std::cout << "FPS: " << static_cast<int>(fps) << " | Frame time: " 
                     << (frameTime * 1000.0 / frameCount) << "ms" << std::endl;
            frameCount = 0;
            frameTime = 0.0;
        }
    }

    // 清理资源
    cleanup();

    std::cout << "Wind Engine terminated successfully" << std::endl;
    return 0;
}
