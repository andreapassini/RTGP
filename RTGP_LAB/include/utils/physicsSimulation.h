#pragma once

#define FIXED_TIME_STEP (1.0f / 30.0f)
#define FIXED_TIME_STEP2 (FIXED_TIME_STEP * FIXED_TIME_STEP)

//#include <utils/cloth.h>
#include <utils/Cloth.h>
#include <vector>

class PhysicsSimulation
{
private:
    double virtualTime;
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

    // void TimeStep(Cloth* cloth, SphereCollider sphere, float yLimit){
    //     if(isPaused) 
    //         return;

    //     cloth->PhysicsSteps(sphere, yLimit);
        
    //     virtualTime += FIXED_TIME_STEP;
    // }

    void TimeStep(){
        if(isPaused) 
            return;
        
        virtualTime += FIXED_TIME_STEP;
    }

    double getVirtualTIme(){ return virtualTime; }
};

