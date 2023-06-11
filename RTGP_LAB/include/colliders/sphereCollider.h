#pragma once

#include <colliders/collider.h>

class SphereCollider: public Collider
{
private:
public:
    explicit SphereCollider(Transform* t, float rad): Collider(t){
        this->radius = rad;
    };
    float radius;
    Transform* transform;
};