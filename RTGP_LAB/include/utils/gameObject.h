#pragma once

#include <Particle.h>
#include <transform.h>

class GameObject
{
private:

public:
    Transform* transform;
    

    GameObject(/* args */);
    ~GameObject();
};
