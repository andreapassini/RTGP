#pragma once

#include <utils/Transform.h>
#include <colliders/collider.h>
#include <glm/glm.hpp>

class CapsuleCollider
{
private:
    /* data */
public:
    float radius;
    Transform* p1;
    Transform* p2;

    CapsuleCollider(Transform* p1Transform, Transform* p2Transform, float radius){
        this->p1 = p1Transform;
        this->p2 = p2Transform;
        this->radius = radius;
    }
};
