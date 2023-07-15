#pragma once

#include <Particle.h>
#include <transform.h>

class GameObject
{
private:

public:
    Particle* particle;
    Transform* transform;

    GameObject* parent;
    GameObject* child[];

    GameObject(/* args */);
    ~GameObject();
};
