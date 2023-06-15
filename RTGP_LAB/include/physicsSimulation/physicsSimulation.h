#pragma once

//#include <utils/cloth.h>
#include "../utils/Cloth.h"
#include "physicObject.h"
#include <vector>

#define FIXED_TIME_STEP (1.0f / 60.0f)
#define FIXED_TIME_STEP2 (FIXED_TIME_STEP * FIXED_TIME_STEP)


class PhysicsSimulation
{
private:
    double virtualTime;
    std::vector<PhysicObject> physicWorld;
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

    void FixedTimeStep(float currTime){
        if(isPaused) 
            return;
        
        virtualTime += FIXED_TIME_STEP;
        //std::cout << "Current time: " << currTime << " > Virtual time: " << virtualTime << std::endl;
        std::vector<PhysicObject>::iterator physObject;
		for(physObject = physicWorld.begin(); physObject != physicWorld.end(); physObject++)
		{
			physObject->FixedTimeStep(); // calculate the position of each particle at the next time step.
		}
    }

    double getVirtualTIme(){ return virtualTime; }

    void AddObjectToPhysicWorld(Transform* t, float mass, bool isStatic){
        PhysicObject p(t, mass, isStatic);
        this->physicWorld.push_back(p);
    }

    void AddForceToAll(glm::vec3 force){
        std::vector<PhysicObject>::iterator physObject;
		for(physObject = physicWorld.begin(); physObject != physicWorld.end(); physObject++)
		{
			physObject->AddForce(force); // calculate the position of each particle at the next time step.
		}

    }
};

