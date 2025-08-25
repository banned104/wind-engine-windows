#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>

// GLFW + GLAD
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// 项目组件
#include "EGL_Component/Component_3DModels/ModelRenderer.hpp"

// 全局变量
static std::atomic<bool> g_is_rendering{false};
static std::thread g_render_thread;
static std::mutex g_renderer_mutex;
static ModelRenderer* g_renderer = nullptr;
static GLFWwindow* g_window = nullptr;

// 触摸事件回调函数
void onTouchDown(float x, float y) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMouseDown(x, y, CameraInteractor::MouseButton::Left);
        g_renderer->requestPick();
    }
}

void onTouchMove(float x, float y) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMouseMove(x, y);
    }
}

void onTouchUp(float x, float y) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMouseUp();
    }
}

void onScroll(float delta) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMouseScroll(delta);
    }
}

// GLFW回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
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
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool isDragging = false;
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
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
}

// 停止渲染
void stopRender() {
    if (g_is_rendering) {
        g_is_rendering = false;
        
        if (g_render_thread.joinable()) {
            g_render_thread.join();
        }
        
        std::lock_guard<std::mutex> lock(g_renderer_mutex);
        if (g_renderer) {
            delete g_renderer;
            g_renderer = nullptr;
        }
    }
}

// 开始渲染
void startRender(const std::string& modelDir, int width, int height) {
    if (g_is_rendering) {
        stopRender();
    }

    g_is_rendering = true;

    g_render_thread = std::thread([=]() {
        ModelRenderer* local_renderer = new ModelRenderer(g_window, modelDir, width, height);
        {
            std::lock_guard<std::mutex> lock(g_renderer_mutex);
            g_renderer = local_renderer;
        }

        while (g_is_rendering && !glfwWindowShouldClose(g_window)) {
            local_renderer->draw();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        // 清理渲染器
        std::lock_guard<std::mutex> lock(g_renderer_mutex);
        if (g_renderer == local_renderer) {
            g_renderer = nullptr;
        }
        delete local_renderer;
    });
}

// 初始化GLFW和GLAD
bool initGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // 设置OpenGL版本和配置
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // 创建窗口
    g_window = glfwCreateWindow(800, 600, "3D Model Viewer", nullptr, nullptr);
    if (!g_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(g_window);

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
    int width, height;
    glfwGetFramebufferSize(g_window, &width, &height);
    glViewport(0, 0, width, height);

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;

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
    std::cout << "Starting 3D Model Viewer..." << std::endl;

    // 初始化GLFW和GLAD
    if (!initGLFW()) {
        std::cerr << "Failed to initialize GLFW/GLAD" << std::endl;
        return -1;
    }

    // 设置模型路径（可以通过命令行参数传入）
    std::string modelDir = "models"; // 默认模型目录
    if (argc > 1) {
        modelDir = argv[1];
    }

    std::cout << "Model directory: " << modelDir << std::endl;
    std::cout << "Press ESC to exit" << std::endl;

    // 获取窗口尺寸
    int width, height;
    glfwGetFramebufferSize(g_window, &width, &height);

    // 开始渲染
    startRender(modelDir, width, height);

    // 主循环
    while (!glfwWindowShouldClose(g_window)) {
        glfwPollEvents();
        
        // 渲染在另一个线程中进行
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // 清理资源
    cleanup();

    std::cout << "Application terminated successfully" << std::endl;
    return 0;
}
