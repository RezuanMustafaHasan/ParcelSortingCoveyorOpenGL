#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

Camera::Camera() { setPreset(1); }

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
    const float velocity = speed_ * deltaTime;
    if (direction == CameraMovement::Forward) position_ += front_ * velocity;
    if (direction == CameraMovement::Backward) position_ -= front_ * velocity;
    if (direction == CameraMovement::Left) position_ -= right_ * velocity;
    if (direction == CameraMovement::Right) position_ += right_ * velocity;
    if (direction == CameraMovement::Up) position_ += worldUp_ * velocity;
    if (direction == CameraMovement::Down) position_ -= worldUp_ * velocity;
}

void Camera::processMouse(float xOffset, float yOffset) {
    yaw_ += xOffset * sensitivity_;
    pitch_ = std::max(-89.0f, std::min(89.0f, pitch_ + yOffset * sensitivity_));
    updateVectors();
}

void Camera::processScroll(float yOffset) {
    zoom_ = std::max(25.0f, std::min(70.0f, zoom_ - yOffset));
}

void Camera::lookAt(const glm::vec3& position, const glm::vec3& target) {
    position_ = position;
    const glm::vec3 direction = glm::normalize(target - position);
    pitch_ = glm::degrees(std::asin(direction.y));
    yaw_ = glm::degrees(std::atan2(direction.z, direction.x));
    updateVectors();
}

void Camera::setPreset(int preset) {
    if (preset == 1) lookAt(glm::vec3(17.5f, 16.0f, 26.0f), glm::vec3(0.0f, 1.6f, 0.0f));
    // A tiny forward offset avoids a view vector exactly parallel to world-up.
    if (preset == 2) lookAt(glm::vec3(0.0f, 36.0f, 0.0f), glm::vec3(0.0f, 0.0f, -0.05f));
    if (preset == 3) lookAt(glm::vec3(17.5f, 8.0f, -2.5f), glm::vec3(0.0f, 2.5f, -2.5f));
}

void Camera::updateVectors() {
    glm::vec3 front;
    front.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    front.y = std::sin(glm::radians(pitch_));
    front.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    front_ = glm::normalize(front);
    right_ = glm::normalize(glm::cross(front_, worldUp_));
    up_ = glm::normalize(glm::cross(right_, front_));
}
