#pragma once

#include <utils/Transform.h>
#include <glm/glm.hpp>

class SphereCollider
{
private:
public:
    Transform* transform;
    float radius;
    SphereCollider(Transform* transform, float rad) {
        this->transform = transform;
        this->radius = rad;
    }; 
};