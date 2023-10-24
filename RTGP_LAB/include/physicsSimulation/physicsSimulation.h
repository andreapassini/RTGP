#pragma once

#include "../utils/Cloth.h"
#include "physicObject.h"
#include "physicsParameters.h"
#include <vector>

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

    void FixedTimeStep(){
        if(isPaused) 
            return;
        
        virtualTime += FIXED_TIME_STEP;

        std::vector<PhysicObject>::iterator physObject;
		for(physObject = physicWorld.begin(); physObject != physicWorld.end(); physObject++)
		{
			physObject->FixedTimeStep(); // calculate the position of each particle at the next time step.
		}
    }

    double getVirtualTIme(){ return virtualTime; }

    void AddObjectToPhysicWorld(Transform* t, float mass, bool isStatic){
        PhysicObject p(*(t->translation), mass, isStatic);
        this->physicWorld.push_back(p);
    }

    void AddForceToAll(glm::vec3 force){
        std::vector<PhysicObject>::iterator physObject;
		for(physObject = physicWorld.begin(); physObject != physicWorld.end(); physObject++)
		{
			physObject->AddForce(force); // calculate the position of each particle at the next time step.
		}

    }

    void AddGravityToAll(glm::vec3 force){
        std::vector<PhysicObject>::iterator physObject;
		for(physObject = physicWorld.begin(); physObject != physicWorld.end(); physObject++)
		{
			physObject->AddForce(force * physObject->mass); // calculate the position of each particle at the next time step.
		}
    }
};

