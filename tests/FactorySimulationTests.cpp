#include "FactorySimulation.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void require(bool condition, const std::string& message) {
    if(!condition) throw std::runtime_error(message);
}
bool exitState(BoxState s) { return s==BoxState::LeftExit || s==BoxState::RightExit; }
bool lowerState(BoxState s) { return s==BoxState::LowerConveyor || s==BoxState::Scanning || s==BoxState::ApproachingGate; }
void run(float speed, float dt, bool manual) {
    FactorySimulation sim;
    sim.adjustSpeed(speed-sim.conveyorSpeed());
    float previousGate=0;
    bool previousLocked=false;
    for(int frame=0;frame<static_cast<int>(240.0f/dt);++frame) {
        if(manual && frame%17==0) sim.testGate();
        sim.update(dt);
        const auto& boxes=sim.parcels();
        bool locked=false;
        for(std::size_t i=0;i<boxes.size();++i) {
            const auto& box=boxes[i];
            if(!box.active) continue;
            require(std::isfinite(box.position.x) && std::isfinite(box.position.y) && std::isfinite(box.position.z),"Non-finite parcel position");
            if(box.state==BoxState::Falling)
                require(box.position.z-box.scale.z*0.5f>=FactoryLayout::UpperEndZ,"Parcel fell through the upper belt");
            if(box.state==BoxState::Dropping)
                require(box.routeDistance-box.scale.z*0.5f>=FactoryLayout::ExitLength,"Parcel fell through the exit belt");
            if(exitState(box.state)) {
                require(box.scanned,"Unscanned parcel entered sorter");
                const bool left=box.destination==Destination::Left;
                require((box.state==BoxState::LeftExit)==left,"Parcel routed to wrong exit");
                const auto expected=FactoryLayout::routePosition(left,box.routeDistance);
                require(std::abs(box.position.x-expected.x)<0.001f && std::abs(box.position.z-expected.z)<0.001f,"Parcel drifted off its conveyor");
                if(box.routeDistance<FactoryLayout::GateClearance-0.05f) {
                    locked=true;
                    require(std::abs(sim.gateAngle()-(left?-FactoryLayout::BranchAngle:FactoryLayout::BranchAngle))<0.11f,"Gate moved while parcel occupied sorter");
                }
                require(std::abs(box.yaw)<=FactoryLayout::BranchAngle+7.0f,"Unstable parcel rotation");
            }
            if(box.state==BoxState::Collected) {
                require(std::abs(box.position.y-(0.30f+box.scale.y*0.5f))<0.001f,"Parcel did not land inside bin");
                require((box.position.x<0)==(box.destination==Destination::Left),"Parcel landed in wrong bin");
            }
            for(std::size_t j=i+1;j<boxes.size();++j) {
                const auto& other=boxes[j];
                if(!other.active) continue;
                const bool sameLane=(lowerState(box.state)&&lowerState(other.state)) || (box.state==BoxState::UpperConveyor && other.state==BoxState::UpperConveyor);
                if(sameLane) require(std::abs(box.position.z-other.position.z)>=(box.scale.z+other.scale.z)*0.5f+0.37f,"Parcels overlapped in queue");
            }
        }
        if(locked && previousLocked) require(std::abs(sim.gateAngle()-previousGate)<0.01f,"Gate lock is not stable");
        previousGate=sim.gateAngle(); previousLocked=locked;
    }
    require(sim.sortedCount(Destination::Left)>12 && sim.sortedCount(Destination::Right)>12,"Sorter stalled or starved a destination");
    std::cout<<"PASS speed="<<speed<<" dt="<<dt<<" manual="<<manual<<" sorted="<<sim.sortedCount(Destination::Left)+sim.sortedCount(Destination::Right)<<'\n';
}
}
int main() {
    try {
        for(float speed:{0.65f,1.55f,3.0f}) for(float dt:{1.0f/144.0f,1.0f/30.0f,0.1f}) run(speed,dt,false);
        run(3.0f,1.0f/60.0f,true);
        FactorySimulation sim;
        for(int i=0;i<800;++i) sim.update(1.0f/60.0f);
        sim.togglePaused();
        const auto before=sim.parcels(); const float gate=sim.gateAngle();
        sim.testGate(); sim.update(0.2f);
        require(sim.gateAngle()==gate,"Paused gate moved");
        for(std::size_t i=0;i<before.size();++i) require(before[i].position==sim.parcels()[i].position,"Paused parcel moved");
        sim.reset();
        require(!sim.paused() && sim.gateAngle()==0 && sim.sortedCount(Destination::Left)==0 && sim.sortedCount(Destination::Right)==0,"Reset did not clear sorter state");
        require(sim.parcels()[0].active && sim.parcels()[0].position.z==-7.55f,"Reset did not seed input parcel");
        // Changing speed while queues are occupied must also remain live.
        for(int i=0;i<18000;++i) { if(i%71==0) sim.adjustSpeed(i%142==0?3.0f:-3.0f); sim.update(1.0f/60.0f); }
        require(sim.sortedCount(Destination::Left)>20 && sim.sortedCount(Destination::Right)>20,"Speed changes stalled the sorter");
        std::cout<<"PASS pause, reset and changing speed\n";
    } catch(const std::exception& error) { std::cerr<<"FAIL: "<<error.what()<<'\n'; return 1; }
}
