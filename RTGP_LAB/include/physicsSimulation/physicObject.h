#pragma once

#include <utils/Transform.h>
#include <glm/glm.hpp>



class PhysicObject
{
private:
public:
    glm::vec3 pos;
    glm::vec3 old_pos;
    glm::vec3 force;
    float mass;
    bool isStatic;
    PhysicObject(glm::vec3 position, float objectMass, bool objectMovable){
        this->pos = position;
        this->force = glm::vec3(0.0f);
        this->mass = objectMass;
        this->old_pos = position;
        this->isStatic = objectMovable;
    };
    ~PhysicObject(){};

    void FixedTimeStep(){
        if(!isStatic){
			this->force = glm::vec3(0.0f);
			return;
		}

		glm::vec3 now_pos = pos;
		glm::vec3 accel = force/mass;
		pos = ((2.0f - DAMPING) * now_pos) - ( (1.0f - DAMPING) * (old_pos)) + (accel * FIXED_TIME_STEP2);
		old_pos = now_pos;
		this->force = glm::vec3(0.0f);
    }

    void AddForce(glm::vec3 forceToAdd){
        this->force += forceToAdd;
    }
};