#pragma once

#include "../colliders/CapsuleCollider.h"
#include "../colliders/PlaneCollider.h"
#include "../colliders/sphereCollider.h"
#include "../utils/renderableObject.h"

#include <vector>

class iScene
{
private:

public:
    virtual void Start() = 0;
    virtual void Update() = 0;
    virtual void FixedUpdate() = 0;
protected:
    vector<PlaneCollider*> planes;
    vector<SphereCollider*> spheres;
    vector<CapsuleCollider*> capsules;

    vector<RenderableObject*> renderableObjects;

};