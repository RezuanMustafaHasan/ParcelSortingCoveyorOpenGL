#include "FactoryScene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {
using namespace FactoryLayout;
const glm::vec3 Steel(0.49f,0.60f,0.62f);
const glm::vec3 DarkSteel(0.075f,0.14f,0.18f);
const glm::vec3 Belt(0.045f,0.09f,0.105f);
const glm::vec3 Yellow(1.0f,0.69f,0.09f);
const glm::vec3 Teal(0.055f,0.38f,0.40f);
const glm::vec3 Blue(0.065f,0.40f,0.73f);
const glm::vec3 Orange(0.95f,0.32f,0.07f);
const glm::vec3 Ivory(0.89f,0.87f,0.75f);
const glm::vec3 Wood(0.56f,0.34f,0.15f);

glm::mat4 transform(const glm::vec3& p, const glm::vec3& r, const glm::vec3& s) {
    glm::mat4 m = glm::translate(glm::mat4(1), p);
    m = glm::rotate(m, glm::radians(r.y), {0,1,0});
    m = glm::rotate(m, glm::radians(r.x), {1,0,0});
    m = glm::rotate(m, glm::radians(r.z), {0,0,1});
    return glm::scale(m, s);
}
// A tiny geometric stencil alphabet keeps signage crisp without external assets.
const char* glyph(char c) {
    switch(c) {
    case 'A': return "010101111101101"; case 'B': return "110101110101110";
    case 'C': return "111100100100111"; case 'D': return "110101101101110";
    case 'E': return "111100110100111"; case 'F': return "111100110100100";
    case 'G': return "111100101101111"; case 'H': return "101101111101101";
    case 'I': return "111010010010111"; case 'J': return "001001001101111";
    case 'K': return "101101110101101"; case 'L': return "100100100100111";
    case 'M': return "101111111101101"; case 'N': return "101111111111101";
    case 'O': return "111101101101111"; case 'P': return "111101111100100";
    case 'Q': return "111101101111001"; case 'R': return "110101110101101";
    case 'S': return "111100111001111"; case 'T': return "111010010010010";
    case 'U': return "101101101101111"; case 'V': return "101101101101010";
    case 'W': return "101101111111101"; case 'X': return "101101010101101";
    case 'Y': return "101101010010010"; case 'Z': return "111001010100111";
    case '0': return "111101101101111"; case '1': return "010110010010111";
    case '2': return "111001111100111"; case '3': return "111001111001111";
    case '4': return "101101111001001"; case '5': return "111100111001111";
    case '6': return "111100111101111"; case '7': return "111001010010010";
    case '8': return "111101111101111"; case '9': return "111101111001111";
    case '-': return "000000111000000"; default: return "000000000000000";
    }
}
}
using namespace FactoryLayout;

FactoryScene::FactoryScene() : cube_(Mesh::cube()), cylinder_(Mesh::cylinder(20)) {}
void FactoryScene::drawCube(Shader& s, const glm::mat4& m, const glm::vec3& c, float shine, float emission) const {
    s.setMat4("model",m); s.setVec3("materialColor",c);
    s.setFloat("shininess",shine); s.setFloat("emissiveStrength",emission); cube_.draw();
}
void FactoryScene::drawCylinder(Shader& s, const glm::mat4& m, const glm::vec3& c, float shine, float emission) const {
    s.setMat4("model",m); s.setVec3("materialColor",c);
    s.setFloat("shininess",shine); s.setFloat("emissiveStrength",emission); cylinder_.draw();
}
void FactoryScene::drawText(Shader& s, const std::string& text, const glm::mat4& parent, float height, const glm::vec3& color, float emission) const {
    const float pixel = height/5.0f;
    const float width = (text.size()*4.0f-1.0f)*pixel;
    for (std::size_t i=0; i<text.size(); ++i) {
        const char* pattern = glyph(text[i]);
        for (int row=0; row<5; ++row) for(int col=0; col<3; ++col)
            if(pattern[row*3+col] == '1')
                drawCube(s,parent*transform({-width*0.5f+(i*4.0f+col+0.5f)*pixel, (2-row)*pixel,0},{0,0,0},{pixel*0.88f,pixel*0.88f,0.012f}),color,12,emission);
    }
}
void FactoryScene::drawParcel(Shader& s, const glm::mat4& parent, const glm::vec3& size, const glm::vec3& color, bool label) const {
    drawCube(s,parent*transform({0,0,0},{0,0,0},size),color,8);
    drawCube(s,parent*transform({0,size.y*0.5f+0.006f,0},{0,0,0},{0.15f,0.012f,size.z+0.008f}),{0.82f,0.67f,0.40f},8);
    drawCube(s,parent*transform({0,0,size.z*0.5f+0.006f},{0,0,0},{0.15f,size.y,0.012f}),{0.82f,0.67f,0.40f},8);
    if (label) {
        drawCube(s,parent*transform({size.x*0.21f,0.04f,size.z*0.5f+0.013f},{0,0,0},{size.x*0.31f,size.y*0.40f,0.012f}),Ivory,8);
        for(int i=0;i<5;++i)
            drawCube(s,parent*transform({size.x*0.10f+i*size.x*0.048f,0.045f,size.z*0.5f+0.021f},{0,0,0},{i%2 ? 0.018f:0.028f,size.y*0.24f,0.006f}),DarkSteel,8);
    }
}
void FactoryScene::drawPallet(Shader& s, const glm::vec3& position, float yaw) const {
    const auto p=transform(position,{0,yaw,0},{1,1,1});
    for(float x:{-0.75f,0.0f,0.75f})
        drawCube(s,p*transform({x,0.12f,0},{0,0,0},{0.18f,0.24f,1.55f}),Wood*0.8f,8);
    for(int i=0;i<6;++i)
        drawCube(s,p*transform({0,0.27f,-0.68f+i*0.27f},{0,0,0},{1.85f,0.09f,0.20f}),Wood*(0.94f+i*0.035f),8);
}
void FactoryScene::render(Shader& s, const glm::vec3& eye) const {
    s.setBool("pointLightEnabled",mainLightEnabled_);
    drawRoom(s,eye); drawConveyors(s); drawScannerAndGate(s); drawBoxes(s); drawBins(s);
    drawControlPanel(s); drawStorageRack(s); drawSafetyFence(s); drawWallFan(s);
    drawPipes(s); drawWorkstations(s); drawLogistics(s); drawUtilities(s);
}

void FactoryScene::drawRoom(Shader& s, const glm::vec3& eye) const {
    s.setBool("floorMaterial",true);
    drawCube(s,transform({0,-0.2f,0},{0,0,0},{20,0.4f,24}),{0.55f,0.51f,0.39f},14);
    s.setBool("floorMaterial",false);
    // Muted teal epoxy work zone and an unobstructed pedestrian aisle.
    drawCube(s,transform({0,0.008f,0.4f},{0,0,0},{9.4f,0.012f,21.8f}),{0.22f,0.36f,0.33f},18);
    for(float x:{-4.82f,4.82f}) drawCube(s,transform({x,0.025f,0.4f},{0,0,0},{0.10f,0.014f,21.8f}),Yellow,8);
    for(float z:{-10.5f,11.3f}) drawCube(s,transform({0,0.026f,z},{0,0,0},{9.75f,0.018f,0.10f}),Yellow,8);
    for(int i=0;i<12;++i) drawCube(s,transform({-4.15f,0.028f,-9.8f+i*1.78f},{0,0,0},{0.07f,0.016f,0.8f}),Ivory,8);
    for(int i=0;i<8;++i) drawCube(s,transform({-6.9f,0.03f,8.3f+i*0.31f},{0,0,0},{3.6f,0.018f,0.15f}),Ivory,8);
    for(float z:{-8.0f,-4.0f,0.0f,4.0f,8.0f})
        drawCube(s,transform({0,0.001f,z},{0,0,0},{19.6f,0.008f,0.018f}),{0.38f,0.36f,0.29f},8);

    drawCube(s,transform({0,4,-11.8f},{0,0,0},{20,8,0.30f}),Ivory,8);
    drawCube(s,transform({0,1.25f,-11.62f},{0,0,0},{20,2.5f,0.08f}),Teal,18);
    drawCube(s,transform({0,2.58f,-11.55f},{0,0,0},{20,0.13f,0.08f}),Yellow,15);
    for(float side:{-1.0f,1.0f}) {
        // Cut away the near wall when inspecting from outside the room.
        const bool visible = side*eye.x < 9.7f;
        if(visible) {
            drawCube(s,transform({side*9.86f,4,0},{0,0,0},{0.28f,8,24}),Ivory*0.95f,8);
            drawCube(s,transform({side*9.69f,1.25f,0},{0,0,0},{0.06f,2.5f,24}),Teal,18);
            drawCube(s,transform({side*9.64f,2.58f,0},{0,0,0},{0.05f,0.13f,24}),Yellow,15);
            for(float z:{-7.0f,-1.0f,5.0f}) {
                drawCube(s,transform({side*9.64f,5.1f,z},{0,0,0},{0.08f,2.4f,3.7f}),DarkSteel,30);
                drawCube(s,transform({side*9.58f,5.1f,z},{0,0,0},{0.04f,2.17f,3.46f}),{0.43f,0.70f,0.78f},75,0.24f);
                drawCube(s,transform({side*9.54f,5.1f,z},{0,0,0},{0.08f,0.08f,3.5f}),Ivory,30);
                drawCube(s,transform({side*9.54f,5.1f,z},{0,0,0},{0.08f,2.2f,0.08f}),Ivory,30);
            }
        }
        for(float z:{-11.35f,-4.0f,3.5f,11.3f}) {
            const float h=visible ? 7.8f : 1.5f;
            drawCube(s,transform({side*9.45f,h*0.5f,z},{0,0,0},{0.30f,h,0.34f}),Teal*0.68f,45);
            drawCube(s,transform({side*9.45f,0.55f,z},{0,0,0},{0.40f,1.1f,0.44f}),Yellow,25);
        }
    }
    for(float x:{-8.9f,0.0f,8.9f}) drawCube(s,transform({x,7.65f,-11.40f},{0,0,0},{0.24f,0.26f,0.5f}),DarkSteel,45);
    if(eye.y < 20.0f) for(float z:{-7.2f,0.0f,7.2f}) {
        drawCube(s,transform({0,7.8f,z},{0,0,0},{18.9f,0.17f,0.15f}),Teal*0.68f,45);
        for(float x:{-4.8f,4.8f}) {
            for(float dx:{-1.0f,1.0f}) drawCylinder(s,transform({x+dx,7.36f,z},{0,0,0},{0.025f,0.72f,0.025f}),DarkSteel,30);
            drawCube(s,transform({x,6.96f,z},{0,0,0},{2.8f,0.16f,0.65f}),DarkSteel,50);
            drawCube(s,transform({x,6.87f,z},{0,0,0},{2.58f,0.035f,0.48f}),{1.0f,0.91f,0.71f},20,mainLightEnabled_?1.3f:0.04f);
        }
    }
    drawCube(s,transform({-0.8f,6.45f,-11.51f},{0,0,0},{8.5f,1.2f,0.12f}),Teal,20);
    drawText(s,"SORTING HALL",transform({-0.8f,6.45f,-11.42f},{0,0,0},{1,1,1}),0.59f,Ivory);
    // Rear loading shutter with individual slats and protective bollards.
    drawCube(s,transform({0,2.25f,-11.46f},{0,0,0},{4.3f,4.5f,0.16f}),DarkSteel,35);
    for(int i=0;i<16;++i) drawCube(s,transform({0,0.20f+i*0.26f,-11.34f},{0,0,0},{3.9f,0.23f,0.10f}),{0.40f,0.55f,0.53f},40);
    drawText(s,"INBOUND",transform({0,4.85f,-11.42f},{0,0,0},{1,1,1}),0.34f,Teal);
    for(float x:{-2.4f,2.4f}) {
        drawCylinder(s,transform({x,0.64f,-10.85f},{0,0,0},{0.22f,1.28f,0.22f}),Yellow,30);
        drawCylinder(s,transform({x,0.88f,-10.85f},{0,0,0},{0.23f,0.18f,0.23f}),DarkSteel,20);
    }
}

void FactoryScene::drawConveyor(Shader& s,const glm::vec3& center,float length,float width,float yaw,const glm::vec3& accent) const {
    const auto p=transform(center,{0,yaw,0},{1,1,1});
    drawCube(s,p*transform({0,-0.24f,0},{0,0,0},{width+0.24f,0.36f,length}),Teal,48);
    drawCube(s,p*transform({0,0.015f,0},{0,0,0},{width,0.17f,length}),Belt,8);
    for(float side:{-1.0f,1.0f}) {
        drawCube(s,p*transform({side*(width*0.5f+0.09f),-0.05f,0},{0,0,0},{0.12f,0.20f,length}),accent,35);
        drawCube(s,p*transform({side*(width*0.5f+0.06f),0.27f,0},{0,0,0},{0.065f,0.07f,length}),Steel,85);
        for(float z:{-length*0.4f,0.0f,length*0.4f}) {
            drawCube(s,p*transform({side*(width*0.5f+0.06f),0.13f,z},{0,0,0},{0.055f,0.30f,0.06f}),Steel,70);
            drawCylinder(s,p*transform({side*(width*0.5f+0.16f),-0.19f,z},{0,0,90},{0.10f,0.035f,0.10f}),Steel,80);
        }
    }
    const int count=std::max(2,static_cast<int>(length/0.43f));
    for(int i=0;i<count;++i) {
        const float offset=std::fmod(rollerAngle_/360.0f*0.43f,0.43f);
        const float z=-length*0.5f+0.05f+i*(length-0.1f)/count+offset;
        if(z<length*0.5f) drawCube(s,p*transform({0,0.104f,z},{0,0,0},{width-0.1f,0.008f,0.022f}),{0.14f,0.21f,0.22f},8);
    }
    for(float z:{-length*0.5f,length*0.5f})
        drawCylinder(s,p*transform({0,-0.05f,z},{0,0,90},{1,1,1})*transform({0,0,0},{0,rollerAngle_,0},{0.23f,width,0.23f}),Steel,90);
    const float h=center.y-0.4f;
    for(float z:{-length*0.35f,length*0.35f}) {
        for(float x:{-width*0.42f,width*0.42f}) {
            drawCube(s,p*transform({x,-0.4f-h*0.5f,z},{0,0,0},{0.14f,h,0.14f}),Teal,45);
            drawCube(s,p*transform({x,-center.y+0.055f,z},{0,0,0},{0.34f,0.11f,0.32f}),DarkSteel,25);
        }
        drawCube(s,p*transform({0,-center.y+0.36f,z},{0,0,0},{width,0.09f,0.09f}),Steel,35);
    }
    drawCylinder(s,p*transform({width*0.5f+0.32f,-0.30f,-length*0.32f},{0,0,90},{0.42f,0.5f,0.42f}),Blue,60);
    for(int i=0;i<5;++i) drawCylinder(s,p*transform({width*0.5f+0.12f+i*0.09f,-0.30f,-length*0.32f},{0,0,90},{0.46f,0.018f,0.46f}),DarkSteel,60);
}
void FactoryScene::drawConveyors(Shader& s) const {
    drawConveyor(s,{0,UpperBeltY,-4.5f},6.7f,2.1f,0,Yellow);
    drawConveyor(s,{0,LowerBeltY,1.48f},5.36f,2.1f,0,Yellow);
    for(bool left:{true,false}) {
        const float start=TurnLength;
        glm::vec3 center=routePosition(left,(start+ExitLength)*0.5f);
        center.y=LowerBeltY;
        drawConveyor(s,center,ExitLength-start,2.0f,left?-BranchAngle:BranchAngle,left?Blue:Orange);
    }
    // Wide transfer deck closes the Y-junction; its roller bed supports the turn.
    drawCube(s,transform({0,LowerBeltY-0.16f,GateZ+0.62f},{0,0,0},{3.1f,0.36f,1.85f}),Teal,50);
    drawCube(s,transform({0,LowerBeltY+0.06f,GateZ+0.62f},{0,0,0},{3.0f,0.08f,1.85f}),Belt,12);
    for(int i=0;i<8;++i) drawCylinder(s,transform({0,LowerBeltY+0.065f,GateZ-0.13f+i*0.22f},{0,0,90},{0.08f,2.95f,0.08f}),Steel,75);
    // Receiving bed is flush with the lower belt, with side guards around the fall.
    for(float x:{-1.2f,1.2f}) {
        for(float z:{-1.15f,0.05f})
            drawCube(s,transform({x,2.85f,z},{0,0,0},{0.11f,2.6f,0.10f}),Yellow,35);
        for(int i=0;i<5;++i) drawCube(s,transform({x,1.8f+i*0.45f,-0.55f},{0,0,0},{0.13f,0.04f,1.3f}),DarkSteel,20);
    }
}

void FactoryScene::drawScannerAndGate(Shader& s) const {
    for(float x:{-1.35f,1.35f}) {
        drawCube(s,transform({x,2.4f,ScannerZ},{0,0,0},{0.28f,2.1f,0.55f}),Teal,45);
        drawCube(s,transform({x,2.45f,ScannerZ+0.29f},{0,0,0},{0.16f,1.3f,0.025f}),Ivory,20);
    }
    drawCube(s,transform({0,3.5f,ScannerZ},{0,0,0},{3.0f,0.40f,0.6f}),Teal,50);
    drawText(s,"SCAN 01",transform({0,3.52f,ScannerZ+0.32f},{0,0,0},{1,1,1}),0.20f,Ivory);
    drawCube(s,transform({0,3.23f,ScannerZ},{0,0,0},{0.6f,0.12f,0.32f}),DarkSteel,55);
    drawCube(s,transform({0,3.16f,ScannerZ},{0,0,0},{0.4f,0.035f,0.2f}),{0.03f,0.76f,0.95f},30,scannerActive_?1.8f:0.2f);
    if(scannerActive_) {
        for(const auto& box:boxes_) if(box.active && box.state==BoxState::Scanning) {
            const float z=ScannerZ+std::sin(box.stateTimer*15)*box.scale.z*0.43f;
            drawCube(s,transform({0,box.position.y+box.scale.y*0.5f+0.015f,z},{0,0,0},{box.scale.x+0.02f,0.015f,0.035f}),{0.03f,0.85f,1.0f},10,1.6f);
        }
    }
    // The pivot sits BELOW the deck; paired guide arms leave a parcel-wide opening.
    const auto gate=transform({0,LowerBeltY,GateZ},{0,gateAngle_,0},{1,1,1});
    drawCylinder(s,transform({0,LowerBeltY-0.48f,GateZ},{0,0,0},{0.60f,0.50f,0.60f}),Orange,65);
    drawCylinder(s,transform({0,LowerBeltY-0.17f,GateZ},{0,0,0},{0.35f,0.32f,0.35f}),Steel,90);
    drawCube(s,gate*transform({0,-0.25f,0.85f},{0,0,0},{0.18f,0.13f,1.7f}),Yellow,55);
    // Articulated guide segments follow the same circular turn as the parcels.
    for(int i=0;i<7;++i) {
        const float distance=0.25f+i*0.29f;
        const float angle=glm::radians(gateAngle_);
        const float localAngle=angle*std::min(distance/TurnLength,1.0f);
        glm::vec3 center(0,LowerBeltY+0.34f,GateZ+distance);
        if(std::abs(angle)>0.001f) {
            const float radius=TurnLength/angle;
            center.x=radius*(1.0f-std::cos(localAngle));
            center.z=GateZ+radius*std::sin(localAngle);
            if(distance>TurnLength) center+=glm::vec3(std::sin(angle),0,std::cos(angle))*(distance-TurnLength);
        }
        const glm::vec3 lateral(std::cos(localAngle),0,-std::sin(localAngle));
        for(float side:{-1.0f,1.0f}) {
            drawCube(s,transform(center+lateral*side*1.07f,{0,glm::degrees(localAngle),0},{0.10f,0.20f,0.32f}),Yellow,55);
            drawCylinder(s,transform(center+lateral*side*1.07f+glm::vec3(0,0.11f,0),{0,0,0},{0.09f,0.04f,0.09f}),Steel,70);
        }
    }
    for(bool left:{true,false}) {
        const auto d=branchDirection(left);
        const auto p=transform(glm::vec3(d.x*3,0.045f,GateZ+d.z*3),{-90,left?-BranchAngle:BranchAngle,0},{1,1,1});
        drawText(s,left?"A":"B",p,0.7f,left?Blue:Orange);
    }
}
void FactoryScene::drawBoxes(Shader& s) const {
    for(const auto& box:boxes_) if(box.active)
        drawParcel(s,transform(box.position,{box.rotationAngle,box.yaw,box.rotationAngle*0.2f},{1,1,1}),box.scale,box.color);
}
void FactoryScene::drawBins(Shader& s) const {
    for(bool left:{true,false}) {
        glm::vec3 position=routePosition(left,ExitLength+0.65f); position.y=0;
        const auto p=transform(position,{0,left?-BranchAngle:BranchAngle,0},{1,1,1});
        const auto color=left?Blue:Orange;
        drawCube(s,p*transform({0,0.21f,0},{0,0,0},{2.28f,0.18f,2.0f}),DarkSteel,25);
        for(float x:{-1.08f,1.08f}) drawCube(s,p*transform({x,0.67f,0},{0,0,0},{0.12f,1.1f,2.0f}),color,40);
        drawCube(s,p*transform({0,0.67f,0.96f},{0,0,0},{2.28f,1.1f,0.12f}),color,40);
        drawCube(s,p*transform({0,0.44f,-0.96f},{0,0,0},{2.28f,0.48f,0.12f}),color,40);
        for(float x:{-0.85f,0.85f}) for(float z:{-0.65f,0.65f}) drawCylinder(s,p*transform({x,0.12f,z},{0,0,90},{0.23f,0.13f,0.23f}),DarkSteel,25);
        for(float x:{-0.85f,-0.42f,0.0f,0.42f,0.85f}) drawCube(s,p*transform({x,0.65f,1.04f},{0,0,0},{0.06f,0.9f,0.055f}),color*0.7f,30);
        drawCube(s,p*transform({0,0.75f,1.08f},{0,0,0},{1.52f,0.45f,0.035f}),Ivory,15);
        drawText(s,left?"A BLUE":"B AMBER",p*transform({0,0.75f,1.11f},{0,0,0},{1,1,1}),0.21f,DarkSteel);
        // Low cargo remains in the far end; the incoming drop area stays free.
        drawParcel(s,p*transform({-0.79f,0.50f,0.47f},{0,8,0},{1,1,1}),{0.35f,0.40f,0.50f},color*0.82f);
    }
}

void FactoryScene::drawControlPanel(Shader& s) const {
    const auto p=transform({6.65f,0,1.1f},{0,-16,0},{1,1,1});
    drawCube(s,p*transform({0,1.4f,0},{0,0,0},{1.9f,2.8f,0.78f}),Ivory,40);
    drawCube(s,p*transform({0,0.18f,0},{0,0,0},{2.05f,0.36f,0.9f}),Teal,35);
    drawCube(s,p*transform({0,1.6f,0.41f},{0,0,0},{1.68f,2.1f,0.05f}),Teal,40);
    drawCube(s,p*transform({0,2.04f,0.45f},{0,0,0},{1.36f,0.76f,0.04f}),DarkSteel,20);
    drawText(s,paused_?"PAUSED":"ONLINE",p*transform({0,2.18f,0.49f},{0,0,0},{1,1,1}),0.13f,{0.24f,0.94f,0.71f},0.5f);
    drawText(s,"A "+std::to_string(sortedCount(Destination::Left)),p*transform({-0.33f,1.90f,0.49f},{0,0,0},{1,1,1}),0.11f,Blue,0.7f);
    drawText(s,"B "+std::to_string(sortedCount(Destination::Right)),p*transform({0.33f,1.90f,0.49f},{0,0,0},{1,1,1}),0.11f,Orange,0.7f);
    for(int i=0;i<3;++i) drawCylinder(s,p*transform({-0.48f+i*0.48f,1.26f,0.47f},{90,0,0},{0.18f,0.09f,0.18f}),i==0?glm::vec3(0.08f,0.74f,0.26f):i==1?Yellow:Orange,40,0.2f);
    drawCube(s,p*transform({0.61f,0.72f,0.49f},{0,0,0},{0.08f,0.27f,0.06f}),Steel,80);
    drawText(s,"CONTROL",p*transform({0,2.65f,0.41f},{0,0,0},{1,1,1}),0.18f,Teal);
    for(int i=0;i<4;++i) drawCube(s,p*transform({0,0.48f+i*0.08f,0.45f},{0,0,0},{0.86f,0.025f,0.02f}),DarkSteel,20);
    drawCylinder(s,transform({2.25f,1.30f,3.15f},{0,0,0},{0.10f,2.6f,0.10f}),Steel,70);
    for(int i=0;i<3;++i) {
        const auto color=i==0?glm::vec3(0.92f,0.10f,0.04f):i==1?Yellow:glm::vec3(0.08f,0.8f,0.32f);
        const bool on=i==0?paused_:i==1?scannerActive_&&!paused_:!paused_&&!scannerActive_;
        drawCylinder(s,transform({2.25f,2.7f+i*0.24f,3.15f},{0,0,0},{0.3f,0.20f,0.3f}),color,50,on?1.0f:0.03f);
    }
}

void FactoryScene::drawStorageRack(Shader& s) const {
    for(const auto& center : {glm::vec3(-6.7f,0,-8.8f),glm::vec3(-6.7f,0,-4.6f),glm::vec3(6.6f,0,-8.8f)}) {
        const auto p=transform(center,{0,0,0},{1,1,1});
        for(float x:{-1.83f,1.83f}) for(float z:{-0.84f,0.84f}) {
            drawCube(s,p*transform({x,2.35f,z},{0,0,0},{0.14f,4.7f,0.14f}),Blue,50);
            drawCube(s,p*transform({x,0.3f,z},{0,0,0},{0.25f,0.6f,0.25f}),Yellow,30);
            for(int i=0;i<7;++i) drawCube(s,p*transform({x,0.9f+i*0.5f,z+0.076f},{0,0,0},{0.045f,0.07f,0.008f}),DarkSteel,15);
        }
        for(float y:{0.5f,1.95f,3.4f}) {
            drawCube(s,p*transform({0,y,0},{0,0,0},{3.8f,0.10f,1.85f}),Wood,15);
            for(float z:{-0.89f,0.89f}) drawCube(s,p*transform({0,y-0.05f,z},{0,0,0},{3.8f,0.20f,0.10f}),Orange,35);
            for(int i=0;i<3;++i) {
                const glm::vec3 size(0.8f+0.05f*i,0.82f+0.09f*(i%2),1.10f);
                drawParcel(s,p*transform({-1.12f+i*1.12f,y+0.06f+size.y*0.5f,0},{0,(i-1)*4.0f,0},{1,1,1}),size,(i==1&&y>3)?Teal:glm::vec3(0.64f+0.07f*i,0.43f+0.04f*i,0.22f+0.035f*i));
            }
        }
        drawCube(s,p*transform({0,4.65f,0.89f},{0,0,0},{1.75f,0.37f,0.06f}),Ivory,10);
        drawText(s,center.x>0?"RACK 03":center.z<-7?"RACK 01":"RACK 02",p*transform({0,4.65f,0.94f},{0,0,0},{1,1,1}),0.20f,Teal);
        for(float x:{-1.83f,1.83f}) for(int i=0;i<2;++i)
            drawCube(s,p*transform({x,1.4f+i*1.6f,0},{52,0,0},{0.05f,2.05f,0.05f}),Steel,45);
    }
}
void FactoryScene::drawSafetyFence(Shader& s) const {
    for(float x:{-2.5f,2.5f}) {
        for(float z:{-7.7f,-5.5f,-3.3f}) {
            drawCube(s,transform({x,0.76f,z},{0,0,0},{0.09f,1.52f,0.09f}),Yellow,30);
            drawCube(s,transform({x,0.06f,z},{0,0,0},{0.3f,0.12f,0.3f}),DarkSteel,20);
        }
        for(float y:{0.48f,1.38f}) drawCube(s,transform({x,y,-5.5f},{0,0,0},{0.09f,0.09f,4.4f}),Yellow,30);
        for(int i=0;i<15;++i) drawCube(s,transform({x,0.94f,-7.6f+i*0.3f},{0,0,0},{0.026f,0.8f,0.026f}),DarkSteel,20);
    }
}
void FactoryScene::drawWallFan(Shader& s) const {
    const glm::vec3 p(6.6f,6.0f,-11.47f);
    drawCube(s,transform(p,{0,0,0},{2.35f,2.35f,0.15f}),Teal,40);
    drawCylinder(s,transform(p+glm::vec3(0,0,0.09f),{90,0,0},{2.12f,0.15f,2.12f}),DarkSteel,30);
    const auto fan=transform(p+glm::vec3(0,0,0.21f),{0,0,fanAngle_},{1,1,1});
    for(int i=0;i<5;++i) drawCube(s,fan*transform({std::sin(i*1.2566f)*0.5f,std::cos(i*1.2566f)*0.5f,0},{0,0,-i*72.0f-20},{0.28f,0.95f,0.07f}),Steel,75);
    drawCylinder(s,transform(p+glm::vec3(0,0,0.29f),{90,0,0},{0.35f,0.18f,0.35f}),Orange,65);
    for(int i=-4;i<=4;++i) drawCube(s,transform(p+glm::vec3(i*0.22f,0,0.40f),{0,0,0},{0.025f,2.0f,0.025f}),Steel,50);
}
void FactoryScene::drawPipes(Shader& s) const {
    for(int i=0;i<3;++i) {
        const float y=3.6f+i*0.43f;
        const auto color=i==0?Orange:i==1?Blue:Teal;
        drawCylinder(s,transform({9.25f,y,-5.2f},{90,0,0},{0.15f,11.5f,0.15f}),color,65);
        drawCylinder(s,transform({8.9f-i*0.36f,y*0.5f,-10.9f},{0,0,0},{0.15f,y,0.15f}),color,65);
        for(float z:{-9.7f,-6.7f,-3.7f,-0.4f}) {
            drawCube(s,transform({9.32f,y,z},{0,0,0},{0.32f,0.08f,0.10f}),Steel,60);
            drawCylinder(s,transform({9.25f,y,z},{90,0,0},{0.22f,0.08f,0.22f}),Steel,70);
        }
    }
    // Overhead cable tray along the rear of the room.
    drawCube(s,transform({0,7.12f,-10.8f},{0,0,0},{18.2f,0.12f,0.40f}),DarkSteel,40);
    for(int i=0;i<25;++i) drawCube(s,transform({-8.8f+i*0.73f,7.22f,-10.8f},{0,0,0},{0.035f,0.22f,0.46f}),Steel,45);
}

void FactoryScene::drawWorkstations(Shader& s) const {
    const auto p=transform({-7.1f,0,-0.7f},{0,0,0},{1,1,1});
    // Packing bench, drawers, pegboard, tools, tape reels and a shipping carton.
    drawCube(s,p*transform({0,1.48f,0},{0,0,0},{3.7f,0.18f,1.45f}),Wood,18);
    for(float x:{-1.5f,1.5f}) drawCube(s,p*transform({x,0.69f,0},{0,0,0},{0.45f,1.38f,1.2f}),Teal,35);
    drawCube(s,p*transform({-0.94f,0.7f,0},{0,0,0},{1.1f,1.2f,1.18f}),Blue,35);
    for(int i=0;i<3;++i) {
        drawCube(s,p*transform({-0.94f,0.36f+i*0.36f,0.61f},{0,0,0},{0.99f,0.30f,0.05f}),Teal,40);
        drawCube(s,p*transform({-0.94f,0.41f+i*0.36f,0.66f},{0,0,0},{0.35f,0.045f,0.05f}),Steel,80);
    }
    drawCube(s,p*transform({0,2.42f,-0.67f},{0,0,0},{3.65f,1.65f,0.12f}),Teal,20);
    for(int col=0;col<14;++col) for(int row=0;row<5;++row)
        drawCube(s,p*transform({-1.6f+col*0.245f,1.88f+row*0.26f,-0.60f},{0,0,0},{0.035f,0.035f,0.008f}),DarkSteel,10);
    for(int i=0;i<6;++i) {
        drawCube(s,p*transform({-1.25f+i*0.47f,2.45f,-0.53f},{0,0,(i%2)*10.0f},{0.065f,0.53f,0.07f}),Steel,80);
        drawCube(s,p*transform({-1.25f+i*0.47f,2.19f,-0.52f},{0,0,0},{0.11f,0.22f,0.09f}),i%2?Orange:Yellow,25);
    }
    drawParcel(s,p*transform({0.6f,1.9f,0.09f},{0,-12,0},{1,1,1}),{0.88f,0.64f,0.8f},{0.72f,0.52f,0.29f});
    drawCylinder(s,p*transform({-0.85f,1.7f,0.15f},{0,0,90},{0.30f,0.19f,0.30f}),Yellow,15);
    drawText(s,"PACKING",p*transform({0,3.01f,-0.59f},{0,0,0},{1,1,1}),0.22f,Ivory);
    // Rolling tool chest with handles, drawers and four casters.
    const auto cart=transform({-7.35f,0,2.25f},{0,8,0},{1,1,1});
    drawCube(s,cart*transform({0,0.78f,0},{0,0,0},{1.5f,1.15f,0.85f}),Orange,40);
    drawCube(s,cart*transform({0,1.4f,0},{0,0,0},{1.65f,0.10f,0.98f}),DarkSteel,20);
    for(int i=0;i<4;++i) {
        drawCube(s,cart*transform({0,0.42f+i*0.25f,0.44f},{0,0,0},{1.32f,0.20f,0.025f}),Orange*0.77f,35);
        drawCube(s,cart*transform({0,0.48f+i*0.25f,0.47f},{0,0,0},{0.72f,0.04f,0.04f}),Steel,70);
    }
    for(float x:{-0.58f,0.58f}) for(float z:{-0.3f,0.3f}) drawCylinder(s,cart*transform({x,0.15f,z},{0,0,90},{0.24f,0.12f,0.24f}),DarkSteel,20);
}

void FactoryScene::drawLogistics(Shader& s) const {
    // Staged pallet loads occupy the perimeter without crossing the sorting lanes.
    for(const auto& position:{glm::vec3(6.8f,0,6.4f),glm::vec3(7.0f,0,9.4f),glm::vec3(-7.0f,0,-10.8f)}) {
        drawPallet(s,position);
        for(int layer=0;layer<2;++layer) for(int i=0;i<2;++i)
            drawParcel(s,transform(position+glm::vec3(-0.46f+i*0.92f,0.65f+layer*0.66f,0),{0,0,0},{1,1,1}),{0.85f,0.64f,1.32f},{0.69f+0.05f*i,0.47f+0.025f*layer,0.25f});
        for(float x:{-0.72f,0.72f}) drawCube(s,transform(position+glm::vec3(x,1.01f,0),{0,0,0},{0.045f,1.42f,1.34f}),Teal,25);
    }
    for(int i=0;i<3;++i) drawPallet(s,{-5.65f,i*0.32f,10.25f});
    // Parked electric forklift: counterweight, tyres, cab, mast and lowered forks.
    const auto f=transform({-6.7f,0,5.4f},{0,-12,0},{1,1,1});
    drawCube(s,f*transform({0,0.68f,0},{0,0,0},{1.5f,0.68f,2.15f}),Yellow,45);
    drawCube(s,f*transform({0,1.13f,-0.7f},{0,0,0},{1.45f,0.50f,0.66f}),Yellow,50);
    drawCube(s,f*transform({0,0.44f,-0.91f},{0,0,0},{1.46f,0.16f,0.3f}),DarkSteel,20);
    for(float x:{-0.77f,0.77f}) for(float z:{-0.67f,0.68f}) {
        drawCylinder(s,f*transform({x,0.41f,z},{0,0,90},{0.76f,0.28f,0.76f}),{0.035f,0.047f,0.052f},8);
        drawCylinder(s,f*transform({x*1.2f,0.41f,z},{0,0,90},{0.36f,0.04f,0.36f}),Steel,60);
    }
    for(float x:{-0.62f,0.62f}) for(float z:{-0.62f,0.61f}) drawCube(s,f*transform({x,1.87f,z},{0,0,0},{0.07f,1.76f,0.07f}),DarkSteel,45);
    drawCube(s,f*transform({0,2.78f,0},{0,0,0},{1.50f,0.13f,1.60f}),Yellow,35);
    drawCube(s,f*transform({0,1.24f,-0.08f},{0,0,0},{0.66f,0.16f,0.59f}),DarkSteel,10);
    drawCube(s,f*transform({0,1.54f,-0.32f},{-8,0,0},{0.64f,0.62f,0.15f}),DarkSteel,10);
    drawCylinder(s,f*transform({0,1.49f,0.43f},{-25,0,0},{0.075f,0.55f,0.075f}),Steel,55);
    drawCylinder(s,f*transform({0,1.75f,0.31f},{-25,0,0},{0.43f,0.05f,0.43f}),DarkSteel,25);
    for(float x:{-0.52f,0.52f}) {
        drawCube(s,f*transform({x,1.45f,1.16f},{-3,0,0},{0.13f,2.8f,0.18f}),DarkSteel,60);
        drawCube(s,f*transform({x,0.19f,1.86f},{0,0,0},{0.20f,0.10f,1.75f}),Steel,70);
        drawCube(s,f*transform({x,0.51f,1.06f},{0,0,0},{0.20f,0.66f,0.11f}),Steel,70);
    }
    for(float y:{0.75f,2.55f}) drawCube(s,f*transform({0,y,1.18f},{0,0,0},{1.17f,0.13f,0.16f}),DarkSteel,55);
    drawCylinder(s,f*transform({0.45f,2.94f,-0.35f},{0,0,0},{0.17f,0.22f,0.17f}),Orange,40,0.5f);
    // Hand pallet truck beside dispatch stock.
    const auto jack=transform({5.8f,0,3.75f},{0,-20,0},{1,1,1});
    for(float x:{-0.38f,0.38f}) drawCube(s,jack*transform({x,0.20f,0.4f},{0,0,0},{0.23f,0.15f,1.9f}),Orange,35);
    drawCube(s,jack*transform({0,0.38f,-0.55f},{0,0,0},{1.05f,0.38f,0.55f}),Orange,35);
    drawCylinder(s,jack*transform({0,0.95f,-0.79f},{-18,0,0},{0.075f,1.3f,0.075f}),DarkSteel,45);
    drawCube(s,jack*transform({0,1.59f,-0.99f},{0,0,0},{0.6f,0.09f,0.09f}),DarkSteel,25);
    for(float x:{-0.4f,0.4f}) drawCylinder(s,jack*transform({x,0.17f,-0.62f},{0,0,90},{0.3f,0.15f,0.3f}),DarkSteel,20);
}

void FactoryScene::drawUtilities(Shader& s) const {
    // Compressor and service tank.
    const auto p=transform({6.65f,0,-4.5f},{0,0,0},{1,1,1});
    drawCylinder(s,p*transform({0,0.9f,0},{0,0,90},{1.1f,2.45f,1.1f}),Teal,65);
    for(float x:{-0.8f,0.8f}) drawCube(s,p*transform({x,0.30f,0},{0,0,0},{0.2f,0.6f,0.65f}),DarkSteel,40);
    drawCube(s,p*transform({0,1.7f,0},{0,0,0},{1.16f,0.56f,0.72f}),Blue,50);
    for(int i=0;i<7;++i) drawCube(s,p*transform({-0.45f+i*0.15f,1.8f,0},{0,0,0},{0.04f,0.56f,0.8f}),Steel,60);
    drawCylinder(s,p*transform({0.9f,1.57f,0},{90,0,0},{0.32f,0.08f,0.32f}),Ivory,50);
    drawCube(s,p*transform({0.9f,1.59f,0.06f},{0,0,-30},{0.024f,0.19f,0.02f}),Orange,20);
    for(int i=0;i<2;++i) {
        const glm::vec3 pos(8.25f,0,-1.75f+i*1.35f);
        drawCylinder(s,transform(pos+glm::vec3(0,0.69f,0),{0,0,0},{0.90f,1.38f,0.90f}),i?Blue:Orange,40);
        for(float y:{0.12f,0.42f,0.98f,1.35f}) drawCylinder(s,transform(pos+glm::vec3(0,y,0),{0,0,0},{0.94f,0.055f,0.94f}),DarkSteel,50);
        drawCube(s,transform(pos+glm::vec3(0,0.72f,0.454f),{0,0,45},{0.25f,0.25f,0.018f}),Ivory,15);
    }
    // Electrical enclosure and fire station on the rear wall.
    for(int i=0;i<2;++i) {
        drawCube(s,transform({-3.55f-i*1.12f,1.58f,-11.31f},{0,0,0},{1.02f,2.85f,0.52f}),Ivory,35);
        drawCube(s,transform({-3.55f-i*1.12f,1.61f,-11.02f},{0,0,0},{0.88f,2.60f,0.03f}),Teal,35);
        drawCube(s,transform({-3.25f-i*1.12f,1.6f,-10.97f},{0,0,0},{0.05f,0.28f,0.055f}),Steel,70);
        drawCube(s,transform({-3.55f-i*1.12f,2.2f,-10.97f},{0,0,45},{0.24f,0.24f,0.02f}),Yellow,15);
    }
    drawCube(s,transform({-8.7f,1.65f,0.85f},{0,0,0},{0.64f,1.35f,0.2f}),{0.76f,0.08f,0.04f},35);
    drawCylinder(s,transform({-8.7f,1.57f,1.06f},{0,0,0},{0.31f,0.83f,0.31f}),{0.93f,0.10f,0.045f},45);
    drawCube(s,transform({-8.7f,2.06f,1.06f},{0,0,0},{0.32f,0.08f,0.13f}),DarkSteel,35);
    drawCube(s,transform({-8.7f,2.68f,0.85f},{0,0,0},{0.70f,0.70f,0.08f}),{0.08f,0.47f,0.23f},20);
    drawCube(s,transform({-8.7f,2.68f,0.90f},{0,0,0},{0.43f,0.12f,0.03f}),Ivory,10);
    drawCube(s,transform({-8.7f,2.68f,0.91f},{0,0,0},{0.12f,0.43f,0.03f}),Ivory,10);
    // Waste station and cones around a small service bay.
    for(int i=0;i<2;++i) {
        const auto c=i?Teal:Blue;
        drawCube(s,transform({8.2f,0.56f,3.0f+i*1.05f},{0,0,0},{0.67f,1.05f,0.7f}),c,25);
        drawCube(s,transform({8.2f,1.11f,3.0f+i*1.05f},{0,0,0},{0.76f,0.10f,0.78f}),DarkSteel,20);
        drawCube(s,transform({8.2f,0.72f,3.37f+i*1.05f},{0,0,0},{0.33f,0.23f,0.025f}),Ivory,15);
    }
    for(float z:{-2.3f,0.1f}) {
        drawCube(s,transform({4.7f,0.05f,z},{0,0,0},{0.5f,0.1f,0.5f}),DarkSteel,15);
        for(int i=0;i<5;++i) drawCylinder(s,transform({4.7f,0.16f+i*0.12f,z},{0,0,0},{0.32f-i*0.048f,0.12f,0.32f-i*0.048f}),i==2?Ivory:Orange,20);
    }
}
