#pragma once

#define FIXED_TIME_STEP (1.0f / 30.0f)
#define FIXED_TIME_STEP2 (FIXED_TIME_STEP * FIXED_TIME_STEP)

//#include <utils/cloth.h>
#include "cloth.h"
#include <vector>

class Sphere{
public:
    Transform t;
    float radius;
    glm::vec3 Position(){
        glm::mat4 modelMat = t.modelMatrix; // column major
        glm::vec3 origin = glm::vec3(
            modelMat[3].x,
            modelMat[3].y,
            modelMat[3].z
        );

        return origin;
    }
};

class PhysicsSimulation
{
private:
    double virtualTime = 0.0f;
    bool isPaused;
public:
    PhysicsSimulation(/* args */)
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

    void TimeStep(Cloth* cloth, std::vector<Sphere> spheres, float yLimit){
        if(isPaused) 
            return;

        //cloth->PhysicsSteps(spheres, yLimit);
        
        virtualTime += FIXED_TIME_STEP;
    }


    bool const isPaused(){ return isPaused; }
    double const getVirtualTIme(){ return virtualTime; }
};

