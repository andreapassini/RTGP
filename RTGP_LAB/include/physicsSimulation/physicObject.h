#pragma once

#include <utils/Transform.h>
#include <glm/glm.hpp>

#define FIXED_TIME_STEP (1.0f / 30.0f)
#define FIXED_TIME_STEP2 (FIXED_TIME_STEP * FIXED_TIME_STEP)


class PhysicObject
{
private:
    Transform* transform;
    glm::vec3 pos;
    glm::vec3 old_pos;
    glm::vec3 force;
    float mass;
    bool isStatic;
public:
    PhysicObject(Transform* t, float objectMass, bool objectMovable){
        this->transform = t;
        this->pos = t->GetTranslationVector();
        this->force = glm::vec3(0.0f);
        this->mass = objectMass;
        this->old_pos = t->GetTranslationVector();
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
		//pos = (((2.0f * now_pos) - old_pos) * (1.0f-DAMPING) + accel * (deltaTime * deltaTime));
		//pos = now_pos + ((now_pos-old_pos) * (1.0f-DAMPING) + accel) * (deltaTime) * TIME_STEPSIZE2;	// newPos = now_pos + speed * deltaTime
		pos = now_pos + (now_pos-old_pos) * (1.0f-DAMPING) + accel * FIXED_TIME_STEP2;	// newPos = now_pos + speed * deltaTime
		//pos = (((2.0f * now_pos) - old_pos) * (1.0f-DAMPING)) + accel * TIME_STEPSIZE2;	// newPos = now_pos + speed * deltaTime
		old_pos = now_pos;
		this->force = glm::vec3(0.0f);
    }

    void AddForce(glm::vec3 forceToAdd){
        this->force += forceToAdd;
    }
};