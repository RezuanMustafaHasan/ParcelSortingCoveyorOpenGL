#pragma once

#include "Geometry.h"
#include "Shader.h"
#include "FactorySimulation.h"
#include <string>

class FactoryScene : public FactorySimulation {
public:
    FactoryScene();
    void render(Shader& shader, const glm::vec3& viewPosition) const;

private:
    Mesh cube_;
    Mesh cylinder_;
    void drawCube(Shader& shader, const glm::mat4& model, const glm::vec3& color,
                  float shininess = 32.0f, float emissive = 0.0f) const;
    void drawCylinder(Shader& shader, const glm::mat4& model, const glm::vec3& color,
                      float shininess = 32.0f, float emissive = 0.0f) const;
    void drawConveyor(Shader& shader, const glm::vec3& center, float length,
                      float width, float yawDegrees, const glm::vec3& accent) const;
    void drawRoom(Shader& shader, const glm::vec3& viewPosition) const;
    void drawConveyors(Shader& shader) const;
    void drawScannerAndGate(Shader& shader) const;
    void drawBoxes(Shader& shader) const;
    void drawBins(Shader& shader) const;
    void drawControlPanel(Shader& shader) const;
    void drawStorageRack(Shader& shader) const;
    void drawSafetyFence(Shader& shader) const;
    void drawWallFan(Shader& shader) const;
    void drawPipes(Shader& shader) const;
    void drawWorkstations(Shader& shader) const;
    void drawLogistics(Shader& shader) const;
    void drawUtilities(Shader& shader) const;
    void drawParcel(Shader& shader, const glm::mat4& parent, const glm::vec3& size,
                    const glm::vec3& color, bool label = true) const;
    void drawPallet(Shader& shader, const glm::vec3& position, float yaw = 0.0f) const;
    void drawText(Shader& shader, const std::string& text, const glm::mat4& parent,
                  float height, const glm::vec3& color, float emissive = 0.0f) const;
};
