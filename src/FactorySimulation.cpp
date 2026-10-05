#include "FactorySimulation.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float Gravity = -9.81f;
float moveToward(float current, float target, float delta) {
    return current + std::clamp(target - current, -delta, delta);
}
bool exiting(const ParcelBox& box) {
    return box.state == BoxState::LeftExit || box.state == BoxState::RightExit;
}
}

namespace FactoryLayout {
glm::vec3 branchDirection(bool left) {
    const float a = glm::radians(left ? -BranchAngle : BranchAngle);
    return {std::sin(a), 0.0f, std::cos(a)};
}
glm::vec3 routePosition(bool left, float distance) {
    const glm::vec3 origin(0, LowerBeltY + BeltSurface, GateZ);
    const glm::vec3 direction = branchDirection(left);
    const float angle = glm::radians(BranchAngle);
    const float radius = TurnLength / angle;
    const float turn = std::min(std::max(distance,0.0f),TurnLength) / radius;
    const float sign = left ? -1.0f : 1.0f;
    const glm::vec3 arc(sign*radius*(1.0f-std::cos(turn)),0,radius*std::sin(turn));
    return origin + arc + direction * std::max(0.0f,distance-TurnLength);
}
float routeYaw(bool left, float distance) {
    return (left ? -BranchAngle : BranchAngle) * std::clamp(distance/TurnLength,0.0f,1.0f);
}
}
using namespace FactoryLayout;

FactorySimulation::FactorySimulation() { reset(); }
void FactorySimulation::reset() {
    boxes_.fill(ParcelBox{});
    paused_ = scannerActive_ = false;
    conveyorSpeed_ = 1.55f;
    spawnTimer_ = rollerAngle_ = fanAngle_ = gateAngle_ = gateTargetAngle_ = manualGateTimer_ = 0.0f;
    spawnSequence_ = 0;
    manualGateLeft_ = false;
    gateOwner_ = -1;
    sortedCounts_.fill(0);
    spawnBox();
}
void FactorySimulation::togglePaused() { paused_ = !paused_; }
void FactorySimulation::toggleMainLight() { mainLightEnabled_ = !mainLightEnabled_; }
void FactorySimulation::adjustSpeed(float amount) { conveyorSpeed_ = std::clamp(conveyorSpeed_ + amount, 0.65f, 3.0f); }
void FactorySimulation::testGate() {
    manualGateLeft_ = !manualGateLeft_;
    manualGateTimer_ = 2.25f; // The demonstration yields to parcels already in the sorter.
}
bool FactorySimulation::spawnBox() {
    for (const auto& box : boxes_)
        if (box.active && box.state == BoxState::UpperConveyor && box.position.z < -5.9f) return false;
    auto slot = std::find_if(boxes_.begin(), boxes_.end(), [](const ParcelBox& b) { return !b.active; });
    if (slot == boxes_.end()) return false;
    static const glm::vec3 sizes[] = {{0.86f,0.70f,0.82f},{1.03f,0.82f,0.90f},{0.78f,0.62f,0.96f},{1.10f,0.76f,0.84f}};
    *slot = ParcelBox{};
    slot->active = true;
    slot->scale = sizes[spawnSequence_ % 4];
    slot->destination = spawnSequence_ % 2 == 0 ? Destination::Left : Destination::Right;
    slot->color = slot->destination == Destination::Left ? glm::vec3(0.08f,0.48f,0.78f) : glm::vec3(0.96f,0.36f,0.075f);
    slot->position = {0, UpperBeltY + BeltSurface + slot->scale.y*0.5f, -7.55f};
    ++spawnSequence_;
    return true;
}
void FactorySimulation::update(float deltaTime) {
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0f) return;
    // Bound long stalls, but preserve elapsed time at normal and low frame rates.
    float remaining = std::min(deltaTime, 0.25f);
    while (remaining > 0.000001f) {
        const float dt = std::min(remaining, 1.0f/120.0f);
        fanAngle_ = std::fmod(fanAngle_ + 150.0f*dt, 360.0f);
        if (!paused_) step(dt);
        remaining -= dt;
    }
}
void FactorySimulation::step(float dt) {
    rollerAngle_ = std::fmod(rollerAngle_ + conveyorSpeed_*190.0f*dt, 360.0f);
    spawnTimer_ += dt;
    if (spawnTimer_ >= 2.5f && spawnBox()) spawnTimer_ = 0.0f;
    manualGateTimer_ = std::max(0.0f, manualGateTimer_ - dt);

    if (gateOwner_ >= 0) {
        const auto& owner = boxes_[static_cast<std::size_t>(gateOwner_)];
        if (!owner.active || (exiting(owner) && owner.routeDistance >= GateClearance) || owner.state == BoxState::Dropping || owner.state == BoxState::Collected)
            gateOwner_ = -1;
    }
    if (gateOwner_ < 0) {
        float nearest = -100.0f;
        for (std::size_t i=0; i<boxes_.size(); ++i) {
            const auto& box = boxes_[i];
            if (box.active && (box.state == BoxState::Scanning || box.state == BoxState::ApproachingGate) && box.position.z > nearest) {
                gateOwner_ = static_cast<int>(i);
                nearest = box.position.z;
            }
        }
    }
    if (gateOwner_ >= 0)
        gateTargetAngle_ = boxes_[static_cast<std::size_t>(gateOwner_)].destination == Destination::Left ? -BranchAngle : BranchAngle;
    else if (manualGateTimer_ > 0.0f)
        gateTargetAngle_ = manualGateLeft_ ? -BranchAngle : BranchAngle;
    // Hold the last route when idle; do not swing back through a departing parcel.
    gateAngle_ = moveToward(gateAngle_, gateTargetAngle_, 85.0f*dt);
    for (auto& box : boxes_) if (box.active) updateBox(box, dt);
    scannerActive_ = std::any_of(boxes_.begin(), boxes_.end(), [](const ParcelBox& b) { return b.active && b.state == BoxState::Scanning; });
}
float FactorySimulation::advanceOnBelt(const ParcelBox& box, float distance, bool upper) const {
    float next = box.position.z + distance;
    for (const auto& ahead : boxes_) {
        if (&ahead == &box || !ahead.active || ahead.position.z <= box.position.z) continue;
        const bool sameBelt = upper ? ahead.state == BoxState::UpperConveyor || ahead.state == BoxState::Falling
            : ahead.state == BoxState::LowerConveyor || ahead.state == BoxState::Scanning || ahead.state == BoxState::ApproachingGate || (exiting(ahead) && ahead.routeDistance < 1.4f);
        if (sameBelt) next = std::min(next, ahead.position.z - (ahead.scale.z + box.scale.z)*0.5f - 0.40f);
    }
    return std::max(box.position.z, next);
}
void FactorySimulation::updateBox(ParcelBox& box, float dt) {
    box.stateTimer += dt;
    const float restY = LowerBeltY + BeltSurface + box.scale.y*0.5f;
    switch (box.state) {
    case BoxState::UpperConveyor: {
        const float releaseZ = UpperEndZ + box.scale.z*0.5f + 0.025f;
        box.position.z = std::min(releaseZ, advanceOnBelt(box, conveyorSpeed_*dt, true));
        bool receiverClear = true;
        for (const auto& other : boxes_) {
            if (&other == &box || !other.active) continue;
            if (other.state == BoxState::Falling || ((other.state == BoxState::LowerConveyor || other.state == BoxState::Scanning) && other.position.z < 0.85f)) receiverClear = false;
        }
        if (box.position.z >= releaseZ && receiverClear) {
            box.state = BoxState::Falling;
            box.stateTimer = 0;
            box.verticalVelocity = 0;
        }
        break;
    }
    case BoxState::Falling:
        box.verticalVelocity += Gravity*dt;
        box.position.y += box.verticalVelocity*dt;
        box.position.z += 0.62f*dt;
        box.rotationAngle = 16.0f*std::sin(std::min(1.0f, box.stateTimer/0.72f)*3.14159265f);
        if (box.position.y <= restY) {
            box.position.y = restY;
            box.rotationAngle = 0;
            box.state = BoxState::LowerConveyor;
            box.stateTimer = 0;
        }
        break;
    case BoxState::LowerConveyor:
        box.position.z = std::min(ScannerZ, advanceOnBelt(box, conveyorSpeed_*dt, false));
        if (box.position.z >= ScannerZ) {
            box.state = BoxState::Scanning;
            box.stateTimer = 0;
        }
        break;
    case BoxState::Scanning:
        if (box.stateTimer >= 0.65f) {
            box.scanned = true;
            box.state = BoxState::ApproachingGate;
            box.stateTimer = 0;
        }
        break;
    case BoxState::ApproachingGate: {
        const float required = box.destination == Destination::Left ? -BranchAngle : BranchAngle;
        const bool ownsGate = gateOwner_ >= 0 && &boxes_[static_cast<std::size_t>(gateOwner_)] == &box;
        const bool ready = ownsGate && std::abs(gateAngle_ - required) < 0.1f;
        const float stop = GateZ - box.scale.z*0.5f - 0.42f;
        box.position.z = std::min(ready ? GateZ : stop, advanceOnBelt(box, conveyorSpeed_*dt, false));
        if (ready && box.position.z >= GateZ) {
            box.state = box.destination == Destination::Left ? BoxState::LeftExit : BoxState::RightExit;
            box.routeDistance = box.stateTimer = 0;
        }
        break;
    }
    case BoxState::LeftExit:
    case BoxState::RightExit: {
        const float releaseDistance = ExitLength + box.scale.z*0.5f + 0.025f;
        box.routeDistance = std::min(releaseDistance, box.routeDistance + conveyorSpeed_*dt);
        box.position = routePosition(box.destination == Destination::Left, box.routeDistance);
        box.position.y = restY;
        box.yaw = routeYaw(box.destination == Destination::Left, box.routeDistance);
        if (box.routeDistance >= releaseDistance) {
            box.state = BoxState::Dropping;
            box.verticalVelocity = 0;
            box.stateTimer = 0;
        }
        break;
    }
    case BoxState::Dropping:
        box.verticalVelocity += Gravity*dt;
        box.position.y += box.verticalVelocity*dt;
        box.position += branchDirection(box.destination == Destination::Left)*0.55f*dt;
        if (box.position.y <= 0.30f + box.scale.y*0.5f) {
            box.position.y = 0.30f + box.scale.y*0.5f;
            box.state = BoxState::Collected;
            box.stateTimer = 0;
            ++sortedCounts_[box.destination == Destination::Left ? 0 : 1];
        }
        break;
    case BoxState::Collected:
        if (box.stateTimer >= 0.65f) box.active = false;
        break;
    }
}
