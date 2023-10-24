#pragma once

#include "../utils/Transform.h"
#include "../utils/model.h"
#include "../components/iComponent.h"

#include "vector"

class GameObject
{
private:

public:
    Transform transform;
    std::vector<IComponent> components;
    GameObject(){
        transform = new Transform();
    }
    GameObject(Transform* t, Model* m){
        this->transform = t;
        this->model = m;
    };
    ~GameObject();
};
