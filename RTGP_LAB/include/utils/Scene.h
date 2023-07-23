#pragma once

#include <colliders/CapsuleCollider.h>
#include <colliders/PlaneCollider.h>
#include <colliders/sphereCollider.h>
#include <utils/gameObject.h>

class Scene
{
private:
public:
    
    vector<PlaneCollider*> planes;
    vector<SphereCollider*> spheres;
    vector<CapsuleCollider*> capsules;

    vector<GameObject*> objects;

    glm::mat4* projection;
    glm::mat4* view;

    Scene(glm::mat4* projectionMatrix, glm::mat4* viewMatrix){
        this->projection = projectionMatrix;
        this->view = viewMatrix;
    }; 
    ~Scene(){
    };
};
