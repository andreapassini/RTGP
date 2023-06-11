#pragma once

#include <colliders/collider.h>

class SphereCollider: public Collider
{
private:
public:
    SphereCollider(Transform* t, float rad)  : transform(t), radius(rad){};
    float radius;
};