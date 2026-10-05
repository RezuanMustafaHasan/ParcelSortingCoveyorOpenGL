#pragma once

#include <glm/glm.hpp>

enum class CameraMovement { Forward, Backward, Left, Right, Up, Down };

class Camera {
public:
    Camera();
    glm::mat4 viewMatrix() const;
    void processKeyboard(CameraMovement direction, float deltaTime);
    void processMouse(float xOffset, float yOffset);
    void processScroll(float yOffset);
    void setPreset(int preset);

    const glm::vec3& position() const { return position_; }
    float zoom() const { return zoom_; }

private:
    glm::vec3 position_{10.5f, 8.0f, 17.0f};
    glm::vec3 front_{0.0f, 0.0f, -1.0f};
    glm::vec3 up_{0.0f, 1.0f, 0.0f};
    glm::vec3 right_{1.0f, 0.0f, 0.0f};
    glm::vec3 worldUp_{0.0f, 1.0f, 0.0f};
    float yaw_ = -120.0f;
    float pitch_ = -19.0f;
    float speed_ = 8.0f;
    float sensitivity_ = 0.10f;
    float zoom_ = 50.0f;

    void updateVectors();
    void lookAt(const glm::vec3& position, const glm::vec3& target);
};
