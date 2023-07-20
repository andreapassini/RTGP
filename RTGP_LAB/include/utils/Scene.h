#pragma once

#include <colliders/CapsuleCollider.h>
#include <colliders/PlaneCollider.h>
#include <colliders/sphereCollider.h>

#include <list>

class Scene
{
private:
public:
    
    std::list<PlaneCollider*> planes;
    std::list<SphereCollider*> spheres;
    std::list<CapsuleCollider*> capsules;

    Scene(){
    }
    ~Scene(){
    };
};
