#pragma once

#include <colliders/CapsuleCollider.h>
#include <colliders/PlaneCollider.h>
#include <colliders/sphereCollider.h>
#include "utils/renderableObject.h"

class Scene
{
private:
public:
    
    vector<PlaneCollider*> planes;
    vector<SphereCollider*> spheres;
    vector<CapsuleCollider*> capsules;

    vector<RenderableObject*> renderableObjects;

    void (*Update)(Scene* thisScene);

    Scene(){};
    ~Scene(){
    };
};
