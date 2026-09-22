#include "FactoryScene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
constexpr float UpperBeltY = 4.0f;
constexpr float LowerBeltY = 1.45f;
constexpr float UpperEndZ = -1.15f;
constexpr float ScannerZ = 2.05f;
constexpr float GateZ = 4.35f;
constexpr float GateLeftAngle = -35.0f;
constexpr float GateRightAngle = 35.0f;
constexpr float Gravity = -7.5f;

const glm::vec3 Steel(0.28f, 0.31f, 0.35f);
const glm::vec3 DarkSteel(0.14f, 0.17f, 0.20f);
const glm::vec3 Belt(0.055f, 0.065f, 0.075f);
const glm::vec3 Yellow(0.92f, 0.62f, 0.08f);
const glm::vec3 Wall(0.53f, 0.56f, 0.58f);

glm::mat4 transform(const glm::vec3& position, const glm::vec3& rotationDegrees,
                    const glm::vec3& scale) {
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationDegrees.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(rotationDegrees.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(rotationDegrees.z), glm::vec3(0, 0, 1));
    return glm::scale(model, scale);
}

float moveToward(float current, float target, float maxDelta) {
    if (std::abs(target - current) <= maxDelta) return target;
    return current + (target > current ? maxDelta : -maxDelta);
}
}

FactoryScene::FactoryScene() : cube_(Mesh::cube()), cylinder_(Mesh::cylinder(24)) {
    reset();
}

void FactoryScene::reset() {
    for (auto& box : boxes_) box = ParcelBox{};
    paused_ = false;
    scannerActive_ = false;
    conveyorSpeed_ = 1.55f;
    spawnTimer_ = 0.0f;
    spawnSequence_ = 0;
    rollerAngle_ = 0.0f;
    fanAngle_ = 0.0f;
    gateAngle_ = 0.0f;
    gateTargetAngle_ = 0.0f;
    manualGateTimer_ = 0.0f;
    spawnBox();
}

void FactoryScene::togglePaused() { paused_ = !paused_; }
void FactoryScene::toggleMainLight() { mainLightEnabled_ = !mainLightEnabled_; }
void FactoryScene::adjustSpeed(float amount) {
    conveyorSpeed_ = std::max(0.65f, std::min(3.0f, conveyorSpeed_ + amount));
}

void FactoryScene::testGate() {
    manualGateLeft_ = !manualGateLeft_;
    gateTargetAngle_ = manualGateLeft_ ? GateLeftAngle : GateRightAngle;
    manualGateTimer_ = 2.25f;
}

void FactoryScene::spawnBox() {
    auto slot = std::find_if(boxes_.begin(), boxes_.end(), [](const ParcelBox& b) { return !b.active; });
    if (slot == boxes_.end()) return;

    static const glm::vec3 colors[] = {
        {0.12f, 0.42f, 0.78f}, {0.92f, 0.42f, 0.08f},
        {0.14f, 0.62f, 0.32f}, {0.92f, 0.72f, 0.12f},
        {0.52f, 0.29f, 0.13f}, {0.72f, 0.16f, 0.10f}
    };
    static const glm::vec3 scales[] = {
        {0.86f, 0.70f, 0.82f}, {1.03f, 0.82f, 0.90f},
        {0.78f, 0.62f, 0.96f}, {1.10f, 0.76f, 0.84f}
    };

    slot->active = true;
    slot->scale = scales[spawnSequence_ % 4];
    slot->color = colors[spawnSequence_ % 6];
    slot->destination = (spawnSequence_ % 2 == 0) ? Destination::Left : Destination::Right;
    slot->position = glm::vec3(0.0f, UpperBeltY + 0.10f + slot->scale.y * 0.5f, -7.55f);
    slot->state = BoxState::UpperConveyor;
    slot->rotationAngle = 0.0f;
    slot->verticalVelocity = 0.0f;
    slot->stateTimer = 0.0f;
    slot->scanned = false;
    ++spawnSequence_;
}

void FactoryScene::update(float deltaTime) {
    const float dt = std::min(deltaTime, 0.05f);
    fanAngle_ = std::fmod(fanAngle_ + 150.0f * dt, 360.0f);
    if (paused_) return;

    rollerAngle_ = std::fmod(rollerAngle_ + conveyorSpeed_ * 190.0f * dt, 360.0f);
    spawnTimer_ += dt;
    const float spawnInterval = std::max(2.7f, 5.0f - conveyorSpeed_ * 0.8f);
    if (spawnTimer_ >= spawnInterval) {
        spawnTimer_ = 0.0f;
        spawnBox();
    }

    scannerActive_ = false;
    for (auto& box : boxes_) if (box.active) updateBox(box, dt);

    if (manualGateTimer_ > 0.0f) {
        manualGateTimer_ -= dt;
    } else {
        ParcelBox* priority = nullptr;
        float bestZ = -std::numeric_limits<float>::max();
        for (auto& box : boxes_) {
            if (box.active && (box.state == BoxState::Scanning || box.state == BoxState::ApproachingGate) && box.position.z > bestZ) {
                priority = &box;
                bestZ = box.position.z;
            }
        }
        gateTargetAngle_ = priority ? (priority->destination == Destination::Left ? GateLeftAngle : GateRightAngle) : 0.0f;
    }
    updateGate(dt);
}

void FactoryScene::updateGate(float dt) {
    gateAngle_ = moveToward(gateAngle_, gateTargetAngle_, 75.0f * dt);
}

void FactoryScene::updateBox(ParcelBox& box, float dt) {
    const float speed = conveyorSpeed_;
    box.stateTimer += dt;
    switch (box.state) {
    case BoxState::UpperConveyor:
        box.position.z += speed * dt;
        if (box.position.z >= UpperEndZ) {
            box.state = BoxState::Falling;
            box.verticalVelocity = -0.3f;
            box.stateTimer = 0.0f;
        }
        break;
    case BoxState::Falling:
        box.verticalVelocity += Gravity * dt;
        box.position.y += box.verticalVelocity * dt;
        box.position.z += speed * 0.34f * dt;
        box.rotationAngle = std::min(24.0f, box.rotationAngle + 28.0f * dt);
        if (box.position.y <= LowerBeltY + 0.10f + box.scale.y * 0.5f) {
            box.position.y = LowerBeltY + 0.10f + box.scale.y * 0.5f;
            box.rotationAngle = 0.0f;
            box.state = BoxState::LowerConveyor;
            box.stateTimer = 0.0f;
        }
        break;
    case BoxState::LowerConveyor:
        box.position.z += speed * dt;
        if (box.position.z >= ScannerZ) {
            box.position.z = ScannerZ;
            box.state = BoxState::Scanning;
            box.stateTimer = 0.0f;
            box.scanned = true;
        }
        break;
    case BoxState::Scanning:
        scannerActive_ = true;
        if (box.stateTimer >= 0.65f) {
            box.state = BoxState::ApproachingGate;
            box.stateTimer = 0.0f;
        }
        break;
    case BoxState::ApproachingGate: {
        const float required = box.destination == Destination::Left ? GateLeftAngle : GateRightAngle;
        const bool gateReady = std::abs(gateAngle_ - required) < 4.0f;
        if (gateReady || box.position.z < GateZ - 0.85f) box.position.z += speed * 0.82f * dt;
        if (box.position.z >= GateZ) {
            box.position.z = GateZ;
            box.state = box.destination == Destination::Left ? BoxState::LeftExit : BoxState::RightExit;
            box.stateTimer = 0.0f;
        }
        break;
    }
    case BoxState::LeftExit:
        box.position += glm::normalize(glm::vec3(-1.0f, 0.0f, 1.28f)) * speed * dt;
        if (box.position.z >= 8.3f) { box.state = BoxState::Collected; box.stateTimer = 0.0f; }
        break;
    case BoxState::RightExit:
        box.position += glm::normalize(glm::vec3(1.0f, 0.0f, 1.28f)) * speed * dt;
        if (box.position.z >= 8.3f) { box.state = BoxState::Collected; box.stateTimer = 0.0f; }
        break;
    case BoxState::Collected:
        if (box.stateTimer > 0.8f) box.active = false;
        break;
    }
}

void FactoryScene::drawCube(Shader& shader, const glm::mat4& model, const glm::vec3& color,
                            float shininess, float emissive) const {
    shader.setMat4("model", model);
    shader.setVec3("materialColor", color);
    shader.setFloat("shininess", shininess);
    shader.setFloat("emissiveStrength", emissive);
    cube_.draw();
}

void FactoryScene::drawCylinder(Shader& shader, const glm::mat4& model, const glm::vec3& color,
                                float shininess, float emissive) const {
    shader.setMat4("model", model);
    shader.setVec3("materialColor", color);
    shader.setFloat("shininess", shininess);
    shader.setFloat("emissiveStrength", emissive);
    cylinder_.draw();
}

void FactoryScene::render(Shader& shader) const {
    shader.setBool("pointLightEnabled", mainLightEnabled_);
    drawRoom(shader);
    drawConveyors(shader);
    drawScannerAndGate(shader);
    drawBoxes(shader);
    drawBins(shader);
    drawControlPanel(shader);
    drawStorageRack(shader);
    drawSafetyFence(shader);
    drawWallFan(shader);
    drawPipes(shader);
}

void FactoryScene::drawRoom(Shader& s) const {
    drawCube(s, transform({0,-0.18f,0},{0,0,0},{20,0.35f,24}), {0.29f,0.31f,0.32f}, 12);
    drawCube(s, transform({0,4,-11.75f},{0,0,0},{20,8,0.35f}), Wall, 10);
    drawCube(s, transform({-9.85f,4,0},{0,0,0},{0.3f,8,24}), {0.46f,0.49f,0.51f}, 10);
    drawCube(s, transform({9.85f,4,0},{0,0,0},{0.3f,8,24}), {0.46f,0.49f,0.51f}, 10);
    for (float x : {-7.0f, 0.0f, 7.0f})
        drawCube(s, transform({x,7.7f,0},{0,0,0},{0.22f,0.22f,23}), DarkSteel, 50);
    for (float x : {-2.0f, 2.0f})
        drawCube(s, transform({x,0.015f,1.2f},{0,0,0},{0.10f,0.025f,17}), Yellow, 12, 0.05f);
    drawCube(s, transform({0,7.35f,0.5f},{0,0,0},{4.0f,0.18f,0.8f}), {0.82f,0.84f,0.78f}, 80, mainLightEnabled_ ? 0.45f : 0.03f);
}

void FactoryScene::drawConveyor(Shader& s, const glm::vec3& center, float length,
                                float width, float yaw, float floorY) const {
    glm::mat4 parent(1.0f);
    parent = glm::translate(parent, center);
    parent = glm::rotate(parent, glm::radians(yaw), glm::vec3(0,1,0));

    drawCube(s, parent * transform({0,-0.22f,0},{0,0,0},{width+0.20f,0.34f,length}), Steel, 72);
    drawCube(s, parent * transform({0,0,0},{0,0,0},{width,0.14f,length}), Belt, 8);
    drawCube(s, parent * transform({-width*0.5f-0.08f,0.28f,0},{0,0,0},{0.13f,0.18f,length}), {0.48f,0.51f,0.53f}, 72);
    drawCube(s, parent * transform({ width*0.5f+0.08f,0.28f,0},{0,0,0},{0.13f,0.18f,length}), {0.48f,0.51f,0.53f}, 72);

    const int rollerCount = std::max(2, static_cast<int>(length / 0.7f));
    for (int i = 0; i <= rollerCount; ++i) {
        const float z = -length*0.5f + length * static_cast<float>(i) / static_cast<float>(rollerCount);
        glm::mat4 roller = parent;
        roller = glm::translate(roller, {0,-0.08f,z});
        roller = glm::rotate(roller, glm::radians(90.0f), glm::vec3(0,0,1));
        roller = glm::rotate(roller, glm::radians(rollerAngle_), glm::vec3(0,1,0));
        roller = glm::scale(roller, {0.18f,width,0.18f});
        drawCylinder(s, roller, {0.24f,0.27f,0.30f}, 90);
    }

    const float legHeight = std::max(0.2f, center.y - floorY - 0.38f);
    for (float z : {-length*0.38f, length*0.38f}) {
        for (float x : {-width*0.42f, width*0.42f}) {
            drawCube(s, parent * transform({x,-0.34f-legHeight*0.5f,z},{0,0,0},{0.16f,legHeight,0.16f}), DarkSteel, 55);
        }
    }
}

void FactoryScene::drawConveyors(Shader& s) const {
    drawConveyor(s, {0,UpperBeltY,-4.45f}, 6.7f, 2.1f, 0.0f, 0.0f);
    drawConveyor(s, {0,LowerBeltY,1.75f}, 5.9f, 2.1f, 0.0f, 0.0f);
    drawConveyor(s, {-1.55f,LowerBeltY,6.35f}, 5.0f, 1.65f, -37.5f, 0.0f);
    drawConveyor(s, { 1.55f,LowerBeltY,6.35f}, 5.0f, 1.65f,  37.5f, 0.0f);

    // A small receiving plate emphasizes that the height change is intentional.
    drawCube(s, transform({0,1.72f,-0.55f},{0,0,0},{2.15f,0.12f,0.85f}), {0.34f,0.38f,0.40f}, 70);
}

void FactoryScene::drawScannerAndGate(Shader& s) const {
    const glm::vec3 scannerSteel(0.16f,0.24f,0.31f);
    drawCube(s, transform({-1.35f,2.55f,ScannerZ},{0,0,0},{0.22f,2.35f,0.30f}), scannerSteel, 70);
    drawCube(s, transform({ 1.35f,2.55f,ScannerZ},{0,0,0},{0.22f,2.35f,0.30f}), scannerSteel, 70);
    drawCube(s, transform({0,3.68f,ScannerZ},{0,0,0},{2.9f,0.22f,0.34f}), scannerSteel, 70);
    drawCube(s, transform({0,3.42f,ScannerZ},{0,0,0},{0.62f,0.18f,0.38f}),
             {0.08f,0.55f,0.98f}, 90, scannerActive_ ? 1.4f : 0.18f);
    if (scannerActive_)
        drawCube(s, transform({0,2.45f,ScannerZ},{0,0,0},{2.15f,0.035f,0.12f}), {0.08f,0.62f,1.0f}, 8, 1.7f);

    // Gate hierarchy: pivot transform -> rotation -> child offset -> scale.
    const glm::vec3 pivot(0.0f, 1.95f, GateZ);
    glm::mat4 gateParent(1.0f);
    gateParent = glm::translate(gateParent, pivot);
    gateParent = glm::rotate(gateParent, glm::radians(gateAngle_), glm::vec3(0,1,0));
    drawCube(s, gateParent * transform({0,0,1.15f},{0,0,0},{0.26f,0.22f,2.3f}), Yellow, 80);
    drawCylinder(s, transform(pivot,{0,0,0},{0.42f,0.75f,0.42f}), {0.18f,0.20f,0.22f}, 90);
}

void FactoryScene::drawBoxes(Shader& s) const {
    for (const auto& box : boxes_) {
        if (!box.active) continue;
        const glm::mat4 model = transform(box.position, {box.rotationAngle,0,box.rotationAngle*0.35f}, box.scale);
        drawCube(s, model, box.color, 20);
        // Reused cube as packing tape makes direction and rotation easy to see.
        drawCube(s, transform(box.position + glm::vec3(0,box.scale.y*0.505f,0),
                              {box.rotationAngle,0,box.rotationAngle*0.35f},
                              {box.scale.x*0.16f,0.012f,box.scale.z*1.01f}), {0.78f,0.68f,0.46f}, 10);
    }
}

void FactoryScene::drawBins(Shader& s) const {
    const auto bin = [&](float x, const glm::vec3& color) {
        const float z = 9.25f;
        drawCube(s, transform({x,0.65f,z+0.75f},{0,0,0},{2.45f,1.3f,0.20f}), color, 24);
        drawCube(s, transform({x-1.12f,0.65f,z},{0,0,0},{0.20f,1.3f,1.7f}), color, 24);
        drawCube(s, transform({x+1.12f,0.65f,z},{0,0,0},{0.20f,1.3f,1.7f}), color, 24);
        drawCube(s, transform({x,0.12f,z},{0,0,0},{2.4f,0.18f,1.7f}), color*0.72f, 24);
    };
    bin(-3.05f, {0.08f,0.32f,0.68f});
    bin( 3.05f, {0.75f,0.22f,0.07f});
}

void FactoryScene::drawControlPanel(Shader& s) const {
    drawCube(s, transform({6.8f,1.65f,2.6f},{0,-18,0},{2.1f,3.1f,0.65f}), {0.16f,0.19f,0.21f}, 65);
    drawCube(s, transform({6.5f,2.25f,2.20f},{0,-18,0},{1.25f,0.75f,0.05f}), {0.04f,0.09f,0.10f}, 25, 0.08f);
    drawCylinder(s, transform({6.08f,1.55f,2.08f},{90,0,0},{0.22f,0.08f,0.22f}),
                 paused_ ? glm::vec3(0.18f,0.04f,0.03f) : glm::vec3(0.05f,0.72f,0.20f), 40, paused_ ? 0.05f : 0.7f);
    drawCylinder(s, transform({6.68f,1.55f,1.89f},{90,0,0},{0.22f,0.08f,0.22f}), {0.82f,0.12f,0.06f}, 40, paused_ ? 0.8f : 0.05f);

    // Industrial stack light.
    drawCube(s, transform({4.65f,1.22f,3.55f},{0,0,0},{0.16f,2.35f,0.16f}), DarkSteel, 50);
    drawCylinder(s, transform({4.65f,2.62f,3.55f},{0,0,0},{0.48f,0.38f,0.48f}), {0.68f,0.04f,0.03f}, 40, paused_ ? 1.2f : 0.05f);
    drawCylinder(s, transform({4.65f,3.05f,3.55f},{0,0,0},{0.48f,0.38f,0.48f}), {0.95f,0.56f,0.03f}, 40, scannerActive_ && !paused_ ? 1.2f : 0.04f);
    drawCylinder(s, transform({4.65f,3.48f,3.55f},{0,0,0},{0.48f,0.38f,0.48f}), {0.04f,0.72f,0.17f}, 40, !paused_ && !scannerActive_ ? 1.0f : 0.04f);
}

void FactoryScene::drawStorageRack(Shader& s) const {
    const glm::vec3 rack(0.22f,0.25f,0.28f);
    for (float x : {-8.4f,-4.7f})
        for (float z : {-8.6f,-7.0f})
            drawCube(s, transform({x,2.05f,z},{0,0,0},{0.16f,4.1f,0.16f}), rack, 60);
    for (float y : {0.65f,1.95f,3.25f})
        drawCube(s, transform({-6.55f,y,-7.8f},{0,0,0},{3.9f,0.14f,1.9f}), rack, 60);
    const glm::vec3 boxColors[] = {{0.55f,0.31f,0.12f},{0.12f,0.42f,0.72f},{0.78f,0.52f,0.08f}};
    for (int shelf = 0; shelf < 2; ++shelf)
        for (int i = 0; i < 3; ++i)
            drawCube(s, transform({-7.7f+i*1.12f,1.12f+shelf*1.30f,-7.75f},{0,0,0},{0.82f,0.78f,0.92f}), boxColors[(shelf+i)%3], 18);
}

void FactoryScene::drawSafetyFence(Shader& s) const {
    const glm::vec3 c(0.92f,0.63f,0.06f);
    for (int i = 0; i < 8; ++i) {
        const float z = 1.0f + i * 0.72f;
        drawCube(s, transform({-3.25f,0.95f,z},{0,0,0},{0.09f,1.9f,0.09f}), c, 30);
    }
    drawCube(s, transform({-3.25f,0.45f,3.52f},{0,0,0},{0.10f,0.10f,5.25f}), c, 30);
    drawCube(s, transform({-3.25f,1.45f,3.52f},{0,0,0},{0.10f,0.10f,5.25f}), c, 30);
}

void FactoryScene::drawWallFan(Shader& s) const {
    const glm::vec3 hubPos(5.8f,5.35f,-11.45f);
    drawCube(s, transform(hubPos,{0,0,0},{4.0f,4.0f,0.16f}), {0.25f,0.28f,0.30f}, 45);
    drawCube(s, transform(hubPos + glm::vec3(0,0,0.10f),{0,0,0},{3.55f,3.55f,0.12f}), {0.08f,0.09f,0.10f}, 25);
    glm::mat4 fan(1.0f);
    fan = glm::translate(fan, hubPos + glm::vec3(0,0,0.25f));
    fan = glm::rotate(fan, glm::radians(fanAngle_), glm::vec3(0,0,1));
    for (int i = 0; i < 4; ++i) {
        glm::mat4 blade = glm::rotate(fan, glm::radians(i*90.0f), glm::vec3(0,0,1));
        blade = blade * transform({0,0.92f,0},{0,0,-18},{0.52f,1.55f,0.10f});
        drawCube(s, blade, {0.43f,0.47f,0.50f}, 72);
    }
    drawCylinder(s, transform(hubPos + glm::vec3(0,0,0.36f),{90,0,0},{0.62f,0.32f,0.62f}), DarkSteel, 85);
}

void FactoryScene::drawPipes(Shader& s) const {
    const glm::vec3 pipe(0.38f,0.42f,0.44f);
    drawCylinder(s, transform({8.75f,3.5f,-6.8f},{0,0,0},{0.38f,6.4f,0.38f}), pipe, 95);
    drawCylinder(s, transform({8.75f,6.65f,-1.6f},{0,0,90},{0.38f,10.4f,0.38f}), pipe, 95);
    drawCylinder(s, transform({7.4f,3.2f,-11.35f},{90,0,0},{0.24f,5.4f,0.24f}), {0.30f,0.34f,0.36f}, 90);
}
