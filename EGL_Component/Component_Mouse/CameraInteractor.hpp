#pragma once

#include "Camera.hpp"
#include  "macros.h"
#include <glm/glm.hpp>
#include <stdexcept>
#include <functional>
#include "CameraInteractor.hpp"

struct WindUBO;

class CameraInteractor {
public:
    // onMove回调函数 解耦代码
    std::function<void(float, float)> onMoveCallback;
    int mPickedID;
    void setOnMoveCallbackFunction( std::function<void(float,float)> inputCallback ) {
        onMoveCallback = inputCallback;
        LOGI( "(CameraInteractor.hpp) -> onMoveCallback is set." );
    }

    glm::vec2 getLastMousePos() const {
        return m_lastMousePos;
    }

    // 定义鼠标按键，以便清晰地处理不同交互
    enum class MouseButton {
        None,
        Left,   // 通常用于轨道旋转 (Orbit)
        Right,  // 通常用于平移 (Pan)
        Middle  // 通常也用于平移 (Pan)
    };

    /**
     * @brief 构造函数
     * @param camera 指向要被控制的 Camera 对象的指针。不能为空。
     */
    explicit CameraInteractor(Camera* camera);

    // --- 输入事件处理接口 ---

    /**
     * @brief 当鼠标/触摸按下时调用
     * @param x 按下点的 x 坐标
     * @param y 按下点的 y 坐标
     * @param button 按下的键
     */
    void onMouseDown(float x, float y, MouseButton button);

    /**
     * @brief 当鼠标/触摸移动时调用
     * @param x 当前点的 x 坐标
     * @param y 当前点的 y 坐标
     */
    void onMouseMove(float x, float y);

    /**
     * @brief 当鼠标/触摸抬起时调用
     */
    void onMouseUp();

    /**
     * @brief 当鼠标滚轮滚动或双指缩放时调用
     * @param delta 滚动的量。正值通常表示放大，负值表示缩小。
     */
    void onMouseScroll(float delta);

    /**
     * @brief 当多点触控（例如双指捏合/旋转）时调用
     * @param x1 第一个触点的 x 坐标
     * @param y1 第一个触点的 y 坐标
     * @param x2 第二个触点的 x 坐标
     * @param y2 第二个触点的 y 坐标
     */
    void onMultiTouch(float x1, float y1, float x2, float y2);

    /**
     * @brief 当缩放手势发生时调用
     * @param scaleFactor 缩放因子
     */
    void onScale(float scaleFactor);

    glm::vec2 getMouseLastPos() const {
        return m_lastMousePos;
    }
    /**
     * @brief 当触控状态发生变化时调用 (安卓端调用) 
     */
    void onTouchStateChange();

private:
    Camera* m_camera; // 指向被控制的相机

    // --- 内部状态管理 ---
    enum class TouchState {
        None,
        SingleFinger, // 单指操作
        MultiFinger   // 多指操作
    };

    TouchState m_touchState = TouchState::None;
    MouseButton m_activeButton = MouseButton::None;
    glm::vec2 m_lastMousePos{0.0f};

    // 多点触控状态
    float m_initialFingerDist = 0.0f; // 两指初始距离
    float m_initialFingerAngle = 0.0f; // 两指初始角度
    float m_lastFingerDist = 0.0f;      // 上一帧两指距离
    float m_lastFingerAngle = 0.0f;     // 上一帧两指角度
    bool m_isFirstMultiTouchFrame = true; // 是否为多点触控的第一帧

    // --- 灵敏度控制 ---
    float m_scrollSensitivity = 0.1f;
};