#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// In charge of managing:
//  - Model Matrix
//      - Rotation
//      - Scale
//      - Translation
//  - Normal Matrix
class Transform
{
private:
    void ResetToIdentity();
    void InverseTranspose(glm::mat4 &viewMatrix);

    void Scale(glm::vec3 &scaleVector);
    void Scale(glm::f32 x, glm::f32 y, glm::f32 z);

    void Rotate(glm::f32 angleInGrades, glm::vec3 &axis);

    void Translate(glm::vec3 &translationVector);
    void Translate(glm::f32 x, glm::f32 y, glm::f32 z);
public:
    glm::mat4 modelMatrix;
    glm::mat3 normalMatrix;
    Transform(glm::mat4 model, glm::mat3 normal);
    Transform();
    ~Transform();

    void Transformation(glm::vec3 &scaleVector,
                        glm::f32 angleInGrades, glm::vec3 &axis,
                        glm::vec3 &translationVector,
                        glm::mat4 &viewMatrix
                        );
};

Transform::Transform(glm::mat4 model, glm::mat3 normal)
{
    this->modelMatrix = model;
    this->normalMatrix = normal;
}
Transform::Transform(){
    this->modelMatrix = glm::mat4(1.0f);
    this->normalMatrix = glm::mat3(1.0f);
}
Transform::~Transform()
{
}

void Transform::Transformation(glm::vec3 &scaleVector,
                        glm::f32 angleInGrades, glm::vec3 &axis,
                        glm::vec3 &translationVector,
                        glm::mat4 &viewMatrix
                        )
{
    ResetToIdentity();
    
    Translate(translationVector);
    Rotate(angleInGrades, axis);
    Scale(scaleVector);

    InverseTranspose(viewMatrix);
}

void Transform::ResetToIdentity(){
    this->modelMatrix = glm::mat4(1.0f);
    this->normalMatrix = glm::mat3(1.0f);
}

void Transform::Scale(glm::vec3 &scaleVector){
    this->modelMatrix = glm::scale(this->modelMatrix, scaleVector);
}
void Transform::Scale(glm::f32 x, glm::f32 y, glm::f32 z){
    this->modelMatrix = glm::scale(this->modelMatrix, glm::vec3(x, y, z));
}

void Transform::Rotate(glm::f32 angleInGrades, glm::vec3 &axis){
    this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(angleInGrades), axis);
}

void Transform::Translate(glm::vec3 &translationVector){
    this->modelMatrix = glm::translate(this->modelMatrix, translationVector);
}
void Transform::Translate(glm::f32 x, glm::f32 y, glm::f32 z){
    this->modelMatrix = glm::translate(this->modelMatrix, glm::vec3(x, y, z));
}

void Transform::InverseTranspose(glm::mat4 &viewMatrix){
    this->normalMatrix = glm::inverseTranspose(glm::mat3(viewMatrix * this->modelMatrix));
}