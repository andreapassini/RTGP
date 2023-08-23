#pragma once

#include <utils/Transform.h>
#include <glm/glm.hpp>

class PlaneCollider
{
private:
public:
    Transform* transform;
    glm::vec3 normal;
    PlaneCollider(Transform* transform, glm::vec3 n){
        this->transform = transform;
        this->normal = n;
    }
};


