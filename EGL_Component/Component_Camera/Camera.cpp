#include "Camera.hpp"
#include <algorithm> // for std::clamp

// 辅助函数：线性插值
template<typename T>
T lerp(T a, T b, float t) {
    return a + t * (b - a);
}

Camera::Camera(glm::vec3 target, float distance)
    : m_target(target),
      m_distance(distance),
      m_yaw(0.0f),
      m_pitch(15.0f), // 初始给一点俯角，看起来更自然
      m_up(0.0f, 1.0f, 0.0f)
{
    // 初始化实际位置为目标位置
    m_targetActual = m_target;
    m_distanceActual = m_distance;
    m_yawActual = m_yaw;
    m_pitchActual = m_pitch;

    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    // 使用实际位置来计算视图矩阵，以实现平滑效果
    return glm::lookAt(m_position, m_targetActual, m_up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
}

void Camera::orbit(float dx, float dy) {
    m_yaw += dx * m_orbitSpeed;
    m_pitch += dy * m_orbitSpeed;
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
}

void Camera::pan(float dx, float dy) {
    // 根据相机当前的右方向和上方向来计算平移
    glm::vec3 panOffset = -m_right * dx * m_panSpeed * m_distanceActual + m_up * dy * m_panSpeed * m_distanceActual;
    m_target += panOffset;
}

void Camera::zoom(float amount) {
    m_distance /= amount;
    m_distance = std::clamp(m_distance, 0.1f, 5000.0f);
}

void Camera::setTarget(const glm::vec3& target) {
    m_target = target;
}

void Camera::setDistance(float distance) {
    m_distance = distance;
}

void Camera::updateAspectRatio(float aspect) {
    m_aspect = aspect;
}

void Camera::update(float deltaTime) {
    if (m_enableDamping) {
        // 使用 lerp 平滑地更新实际值
        float t = 1.0f - pow(m_dampingFactor, deltaTime);
        m_yawActual = lerp(m_yawActual, m_yaw, t);
        m_pitchActual = lerp(m_pitchActual, m_pitch, t);
        m_distanceActual = lerp(m_distanceActual, m_distance, t);
        m_targetActual = lerp(m_targetActual, m_target, t);
    } else {
        // 如果禁用平滑，则直接赋值
        m_yawActual = m_yaw;
        m_pitchActual = m_pitch;
        m_distanceActual = m_distance;
        m_targetActual = m_target;
    }
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    // 使用球面坐标计算相机位置
    float yawRad = glm::radians(m_yawActual);
    float pitchRad = glm::radians(m_pitchActual);

    m_position.x = m_targetActual.x + m_distanceActual * cos(pitchRad) * sin(yawRad);
    m_position.y = m_targetActual.y + m_distanceActual * sin(pitchRad);
    m_position.z = m_targetActual.z + m_distanceActual * cos(pitchRad) * cos(yawRad);

    // 更新前、右、上向量
    m_forward = glm::normalize(m_targetActual - m_position);
    m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_up = glm::normalize(glm::cross(m_right, m_forward));
}