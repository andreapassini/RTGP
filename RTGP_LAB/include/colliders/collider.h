#pragma once

#include <glm/glm.hpp>
#include <utils/transform.h>

class Collider
{
private:
    Transform* transform;
public:
    Collider(Transform* t)  : transform(t){};
    ~Collider();

    virtual glm::vec3 Collision(glm::vec3 posWorldSpace, float radius) = 0;
};