#pragma once

class SphereCollider
{
private:
public:
    explicit SphereCollider(Transform* t, float rad) {
        this->transform = transform;
        this->radius = rad;
    };
    float radius;
    Transform* transform;
};