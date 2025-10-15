#include <jni.h>
#include <random>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>


#include "EGL_Component/Component_3DModels/ModelRenderer.hpp"
#include "EGL_Component/Component_Camera/Camera.hpp"
#include "EGL_Component/Component_Mouse/CameraInteractor.hpp"

#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <memory>

#define LOGD(...) __android_log_print(ANDROID_LOG_WARN,"WindEngine",__VA_ARGS__)

// 使用一个全局指针来持有我们的渲染器实例
static ModelRenderer* g_renderer = nullptr;
// 使用一个全局线程对象，方便管理
static std::thread g_render_thread;
// 使用一个原子布尔值来控制渲染循环
static std::atomic<bool> g_is_rendering(false);
// 用于保护 g_renderer 的互斥锁
static std::mutex g_renderer_mutex;

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_learnkotlin_MainActivity_stop_1render(JNIEnv *env, jobject thiz) {
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

JNIEXPORT void JNICALL
Java_com_example_learnkotlin_MainActivity_draw_1wind_1test(JNIEnv *env, jobject thiz, jobject surface, jint w, jint h, jstring path) {
    if (g_is_rendering) {
        Java_com_example_learnkotlin_MainActivity_stop_1render(env, thiz);
    }

    const char *model_dir_path = env->GetStringUTFChars(path, nullptr);
    std::string model_dir_str = model_dir_path;
    env->ReleaseStringUTFChars(path, model_dir_path);

    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    g_is_rendering = true;

    g_render_thread = std::thread([=]() {
        ModelRenderer* local_renderer = new ModelRenderer(nativeWindow, model_dir_str, w, h);
        {
            std::lock_guard<std::mutex> lock(g_renderer_mutex);
            g_renderer = local_renderer;
        }

        while (g_is_rendering) {
            local_renderer->draw();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        // Renderer is deleted in stop_render
    });
}

JNIEXPORT void JNICALL
Java_com_example_learnkotlin_MainActivity_onTouchDown(JNIEnv *env, jobject thiz, jfloat x, jfloat y) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMouseDown(x, y, CameraInteractor::MouseButton::Left);
        g_renderer->requestPick();
    }
}

JNIEXPORT void JNICALL
Java_com_example_learnkotlin_MainActivity_onTouchMove(JNIEnv *env, jobject thiz, jfloat x, jfloat y) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMouseMove(x, y);
    }
}

JNIEXPORT void JNICALL
Java_com_example_learnkotlin_MainActivity_onTouchUp(JNIEnv *env, jobject thiz) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMouseUp();
    }
}

JNIEXPORT void JNICALL
Java_com_example_learnkotlin_MainActivity_onMultiTouch(JNIEnv *env, jobject thiz, jfloat x1, jfloat y1, jfloat x2, jfloat y2) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onMultiTouch(x1, y1, x2, y2);
    }
}

// 添加新的JNI方法 防止触控状态变化时导致相机旋转突变
JNIEXPORT void JNICALL
Java_com_example_learnkotlin_MainActivity_onTouchStateChange(JNIEnv *env, jobject thiz) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onTouchStateChange();
    }
}

JNIEXPORT void JNICALL
Java_com_example_learnkotlin_MainActivity_onScale(JNIEnv *env, jobject thiz, jfloat scale_factor) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer && g_renderer->getInteractor()) {
        g_renderer->getInteractor()->onScale(scale_factor);
    }
}

JNIEXPORT void JNICALL
Java_com_example_learnkotlin_MainActivity_setBoundingBoxVisible(JNIEnv *env, jobject thiz, jboolean visible) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer) {
        g_renderer->setBoundingBoxVisible(visible);
    }
}

JNIEXPORT jboolean JNICALL
Java_com_example_learnkotlin_MainActivity_isBoundingBoxVisible(JNIEnv *env, jobject thiz) {
    std::lock_guard<std::mutex> lock(g_renderer_mutex);
    if (g_renderer) {
        return g_renderer->isBoundingBoxVisible();
    }
    return false;
}

} // extern "C"