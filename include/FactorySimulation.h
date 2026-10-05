#pragma once

#include <glm/glm.hpp>
#include <array>
#include <cstddef>

// Rendering and motion share these dimensions, so routes cannot drift off belts.
namespace FactoryLayout {
constexpr float UpperBeltY = 4.0f;
constexpr float LowerBeltY = 1.45f;
constexpr float BeltSurface = 0.10f;
constexpr float UpperEndZ = -1.15f;
constexpr float ScannerZ = 1.75f;
constexpr float GateZ = 4.0f;
constexpr float BranchAngle = 37.5f;
constexpr float TurnLength = 1.8f;
constexpr float ExitLength = 5.5f;
constexpr float GateClearance = 2.75f;
glm::vec3 branchDirection(bool left);
glm::vec3 routePosition(bool left, float distance);
float routeYaw(bool left, float distance);
}

enum class BoxState { UpperConveyor, Falling, LowerConveyor, Scanning,
                      ApproachingGate, LeftExit, RightExit, Dropping, Collected };
enum class Destination { Left, Right };

struct ParcelBox {
    glm::vec3 position{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 color{0.6f};
    float rotationAngle = 0.0f;
    float yaw = 0.0f;
    float verticalVelocity = 0.0f;
    float stateTimer = 0.0f;
    float routeDistance = 0.0f;
    BoxState state = BoxState::UpperConveyor;
    Destination destination = Destination::Left;
    bool scanned = false;
    bool active = false;
};

class FactorySimulation {
public:
    FactorySimulation();
    void update(float deltaTime);
    void reset();
    void togglePaused();
    void adjustSpeed(float amount);
    void testGate();
    void toggleMainLight();
    bool paused() const { return paused_; }
    bool mainLightEnabled() const { return mainLightEnabled_; }
    float conveyorSpeed() const { return conveyorSpeed_; }
    const auto& parcels() const { return boxes_; }
    float gateAngle() const { return gateAngle_; }
    unsigned int sortedCount(Destination destination) const { return sortedCounts_[destination == Destination::Left ? 0 : 1]; }

protected:
    static constexpr std::size_t MaxBoxes = 12;
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
    int gateOwner_ = -1;
    std::array<unsigned int, 2> sortedCounts_{};

private:
    bool spawnBox();
    void step(float dt);
    void updateBox(ParcelBox& box, float dt);
    float advanceOnBelt(const ParcelBox& box, float distance, bool upper) const;
};
