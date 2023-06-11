#pragma once

#define FIXED_TIME_STEP (1.0f / 30.0f)
#define FIXED_TIME_STEP2 (FIXED_TIME_STEP * FIXED_TIME_STEP)

//#include <utils/cloth.h>
#include "cloth.h"
#include <vector>

class PhysicsSimulation
{
private:
    double virtualTime = 0.0f;
public:
    bool isPaused;
    PhysicsSimulation()
    {
    }

    ~PhysicsSimulation()
    {
    }

    void StartPhysicsSimulation(double startingTime){
        virtualTime = startingTime;
        isPaused = false;
    }
    void Pause(){ 
        isPaused = true; 
    }
    void SynchVirtualTime(double timeToSync){
        virtualTime = timeToSync;
    }

    void TimeStep(Cloth* cloth, std::vector<SphereCollider> spheres, float yLimit){
        if(isPaused) 
            return;

        cloth->PhysicsSteps(spheres, yLimit);
        
        virtualTime += FIXED_TIME_STEP;
    }

    double getVirtualTIme(){ return virtualTime; }
};

