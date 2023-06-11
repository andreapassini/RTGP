#pragma once

#include <glm/glm.hpp>
#include <utils/transform.h>

class Collider
{
private:
public:
    Collider(Transform* t)  : transform(t){};
    ~Collider();
    Transform* transform;
};