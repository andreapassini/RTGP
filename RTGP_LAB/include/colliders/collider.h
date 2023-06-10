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
    glm::vec3 GetPositionWorld(){
    }
};