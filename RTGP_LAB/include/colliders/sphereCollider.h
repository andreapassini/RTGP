#pragma once

#include <utils/Transform.h>
#include <glm/glm.hpp>

class SphereCollider
{
private:
public:
    float radius;
    Transform* transform;
    SphereCollider(Transform* transform, float rad) {
        this->transform = transform;
        this->radius = rad;
    }; 
};