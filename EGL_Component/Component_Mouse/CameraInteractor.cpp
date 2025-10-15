#include "CameraInteractor.hpp"

#include "macros.h"

#ifdef _WIN32
#include "corecrt_math_defines.h"
#else
#include <cmath> // For M_PI on non-Windows platforms
#endif

CameraInteractor::CameraInteractor(Camera* camera)
    : m_camera(camera) {
    if (!m_camera) {
        throw std::invalid_argument("CameraInteractor requires a valid Camera pointer.");
    }
}

void CameraInteractor::onMouseDown(float x, float y, MouseButton button) {
    m_touchState = TouchState::SingleFinger;
    m_activeButton = button;
    m_lastMousePos = {x, y};
}

void CameraInteractor::onMouseUp() {
    m_touchState = TouchState::None;
    m_activeButton = MouseButton::None;
    m_isFirstMultiTouchFrame = true;        // 重置多点触控状态

    // 不再重置实例偏移，让每个实例保持在最后被移动到的位置
    // 只清除选中状态，但不重置偏移值
    mPickedID = BACKGROUND_ID;  // 重置为背景ID
}

void CameraInteractor::onMouseMove(float x, float y) {
    if (m_touchState != TouchState::SingleFinger || !m_camera) {
        return;
    }

    glm::vec2 currentPos = {x, y};
    glm::vec2 delta = currentPos - m_lastMousePos;

    // 根据按下的键执行不同操作
    switch (m_activeButton) {
        case MouseButton::Left:
            // 左键拖拽 -> 轨道旋转或实例移动
            if ( mPickedID == BACKGROUND_ID ) {
                // 没有选中实例，执行相机轨道旋转
                m_camera->orbit(-delta.x, delta.y);
            } else {
                // 选中了实例，调用回调函数更新deltaX和deltaY
                if (onMoveCallback) {
                    onMoveCallback(delta.x, delta.y);
                }
            }
            break;
        
        case MouseButton::Right:
        case MouseButton::Middle:
            // 右键或中键拖拽 -> 平移
            if ( mPickedID == BACKGROUND_ID )
            {m_camera->pan(delta.x, delta.y);}
            break;

        case MouseButton::None:
        default:
            break;
    }

    // 更新最后的位置以备下次计算 delta
    m_lastMousePos = currentPos;
}

void CameraInteractor::onMouseScroll(float delta) {
    if (!m_camera) {
        return;
    }
    // 将滚轮的 delta 值转换为一个缩放因子
    // 例如，delta 为 1.0f, 因子为 1.1; delta 为 -1.0f, 因子为 0.9
    float zoomFactor = 1.0f + delta * m_scrollSensitivity;
    
    // 调用 Camera 的 zoom 方法
    m_camera->zoom(zoomFactor);
}

// --- 辅助函数 ---
namespace {
    float distance(float x1, float y1, float x2, float y2) {
        return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
    }

    float angle(float x1, float y1, float x2, float y2) {
        return std::atan2(y2 - y1, x2 - x1);
    }
}

void CameraInteractor::onMultiTouch(float x1, float y1, float x2, float y2) {
    if (!m_camera) return;

    float currentDist = distance(x1, y1, x2, y2);
    float currentAngle = angle(x1, y1, x2, y2);

    if (m_touchState != TouchState::MultiFinger) {
        // 第一次进入多点触控模式
        m_touchState = TouchState::MultiFinger;
        m_initialFingerDist = currentDist;
        m_initialFingerAngle = currentAngle;

        m_lastFingerDist = currentDist;
        m_lastFingerAngle = currentAngle;
        m_isFirstMultiTouchFrame = false;

    } else {
        // --- 缩放 ---
        if (m_initialFingerDist > 0) {
            float scale = currentDist / m_initialFingerDist;
            m_camera->zoom(scale);
        }

        // --- 旋转 ---
        float angleDelta = currentAngle - m_initialFingerAngle;
        // 处理角度跳变
        if ( angleDelta > M_PI ) {
            angleDelta -= 2*M_PI;
        } else if ( angleDelta < -M_PI ) {
            angleDelta += 2*M_PI;
        }
        // 将角度转换为相机轨道旋转的输入 只有角度变化足够大才会旋转
        if ( std::abs( angleDelta ) > 0.01f ) {
            m_camera->orbit(glm::degrees(angleDelta) * 0.5f, 0);
        }
            

        // 更新初始状态以进行下一次增量计算
        m_initialFingerDist = currentDist;
        m_initialFingerAngle = currentAngle;
    }

    // 更新最后的位置，尽管在多点触控中可能不直接使用
    m_lastMousePos = {(x1 + x2) / 2.0f, (y1 + y2) / 2.0f};
}

void CameraInteractor::onTouchStateChange() {
    // 安卓端通知触控状态变化发生时重置多点触控状态
    m_isFirstMultiTouchFrame = true;
}

void CameraInteractor::onScale(float scaleFactor) {
    if (!m_camera) {
        return;
    }
    // Invert the scale factor to match user expectation
    // scaleFactor > 1 means zoom in (fingers apart), so we need a zoom value < 1 to decrease distance.
    if (scaleFactor > 0.0f) {
        m_camera->zoom( scaleFactor);
    }
}