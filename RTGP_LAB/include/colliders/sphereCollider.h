#include <colliders/collider.h>

class sphereCollider: public Collider
{
private:
    float radius;
public:
    sphereCollider(/* args */);
    ~sphereCollider();

    glm::vec3 Collision(glm::vec3 posWorldSpace, float radius){
        
    }
};