#pragma once

#include <utils/Transform.h>
#include <glm/glm.hpp>

class PlaneCollider
{
private:
public:
    Transform* transform;
    glm::vec3 normal;
    PlaneCollider(Transform* transform){
        this->transform = transform;
        this->normal = transform->rotation->Apply(glm::vec3(0.0f, 1.0f, 0.0f));
    }
};


