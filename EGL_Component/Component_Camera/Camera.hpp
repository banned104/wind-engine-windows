#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm> // for std::clamp

class Camera {
public:
    Camera(glm::vec3 target = glm::vec3(0.0f), float distance = 10.0f);

    // --- 核心矩阵获取 ---
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    // --- 用户输入处理 ---
    void orbit(float dx, float dy);
    void pan(float dx, float dy);
    void zoom(float amount);

    // --- 属性设置 ---
    void setTarget(const glm::vec3& target);
    void setDistance(float distance);
    void updateAspectRatio(float aspect);

    // --- 状态获取 ---
    const glm::vec3& getPosition() const { return m_position; }
    const glm::vec3& getTarget() const { return m_target; }
    glm::vec3 getForward() const { return glm::normalize(m_target - m_position); }

    // --- 更新循环 (用于平滑移动) ---
    void update(float deltaTime);

private:
    void updateCameraVectors();

    // --- 相机核心属性 ---
    glm::vec3 m_position; // 相机在世界空间中的位置
    glm::vec3 m_target;   // 相机观察的目标点
    glm::vec3 m_up;       // 世界空间的上方向
    glm::vec3 m_right;    // 相机的右方向
    glm::vec3 m_forward;  // 相机的朝向

    // --- 轨道控制属性 ---
    float m_distance; // 距离目标的距离
    float m_yaw;      // 水平角（偏航角）
    float m_pitch;    // 垂直角（俯仰角）

    // --- 投影属性 ---
    float m_fov = 45.0f;
    float m_aspect = 16.0f / 9.0f;  // 宽高比
    float m_near = 0.1f;
    float m_far = 5000.0f;

    // --- 灵敏度控制 ---
    float m_orbitSpeed = 0.25f;
    float m_zoomSpeed = 0.05f;
    float m_panSpeed = 0.01f;

    // --- 平滑移动 (Damping) ---
    bool m_enableDamping = false;
    float m_dampingFactor = 0.15f;
    glm::vec3 m_targetActual;   // 实际的目标点 (用于插值)
    float m_distanceActual; // 实际的距离 (用于插值)
    float m_yawActual;      // 实际的yaw (用于插值)
    float m_pitchActual;    // 实际的pitch (用于插值)
};