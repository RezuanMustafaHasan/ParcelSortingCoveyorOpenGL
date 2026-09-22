#pragma once

#include "Geometry.h"
#include "Shader.h"

#include <glm/glm.hpp>
#include <array>

enum class BoxState {
    UpperConveyor,
    Falling,
    LowerConveyor,
    Scanning,
    ApproachingGate,
    LeftExit,
    RightExit,
    Collected
};

enum class Destination { Left, Right };

struct ParcelBox {
    glm::vec3 position{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 color{0.6f};
    float rotationAngle = 0.0f;
    float verticalVelocity = 0.0f;
    float stateTimer = 0.0f;
    BoxState state = BoxState::UpperConveyor;
    Destination destination = Destination::Left;
    bool scanned = false;
    bool active = false;
};

class FactoryScene {
public:
    FactoryScene();

    void update(float deltaTime);
    void render(Shader& shader) const;
    void reset();
    void togglePaused();
    void adjustSpeed(float amount);
    void testGate();
    void toggleMainLight();

    bool paused() const { return paused_; }
    bool mainLightEnabled() const { return mainLightEnabled_; }
    float conveyorSpeed() const { return conveyorSpeed_; }

private:
    static constexpr std::size_t MaxBoxes = 8;
    Mesh cube_;
    Mesh cylinder_;
    std::array<ParcelBox, MaxBoxes> boxes_{};

    bool paused_ = false;
    bool mainLightEnabled_ = true;
    bool scannerActive_ = false;
    float conveyorSpeed_ = 1.55f;
    float spawnTimer_ = 0.0f;
    unsigned int spawnSequence_ = 0;
    float rollerAngle_ = 0.0f;
    float fanAngle_ = 0.0f;
    float gateAngle_ = 0.0f;
    float gateTargetAngle_ = 0.0f;
    float manualGateTimer_ = 0.0f;
    bool manualGateLeft_ = false;

    void spawnBox();
    void updateBox(ParcelBox& box, float dt);
    void updateGate(float dt);

    void drawCube(Shader& shader, const glm::mat4& model, const glm::vec3& color,
                  float shininess = 32.0f, float emissive = 0.0f) const;
    void drawCylinder(Shader& shader, const glm::mat4& model, const glm::vec3& color,
                      float shininess = 32.0f, float emissive = 0.0f) const;
    void drawConveyor(Shader& shader, const glm::vec3& center, float length,
                      float width, float yawDegrees, float floorY) const;
    void drawRoom(Shader& shader) const;
    void drawConveyors(Shader& shader) const;
    void drawScannerAndGate(Shader& shader) const;
    void drawBoxes(Shader& shader) const;
    void drawBins(Shader& shader) const;
    void drawControlPanel(Shader& shader) const;
    void drawStorageRack(Shader& shader) const;
    void drawSafetyFence(Shader& shader) const;
    void drawWallFan(Shader& shader) const;
    void drawPipes(Shader& shader) const;
};
