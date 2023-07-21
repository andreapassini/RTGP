#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "quaternion.h"

// In charge of managing:
//  - Model Matrix
//      - Rotation
//      - Scale
//      - Translation
//  - Normal Matrix
class Transform
{
private:
        
public:
    glm::mat4 modelMatrix;
    glm::mat3 normalMatrix;
    glm::mat4 viewMatrix;

    glm::vec3 translation;
    Quaternion rotation;
    float scale;

    Transform::Transform(){
        this->viewMatrix = glm::mat4(1.0f);
        this->modelMatrix = glm::mat4(1.0f);
        this->normalMatrix = glm::mat3(1.0f);
    }

    Transform::Transform(glm::mat4 &model, glm::mat3 &normal, glm::mat4 &viewMatrix)
    {
        this->modelMatrix = model;
        this->normalMatrix = normal;
        this->viewMatrix = viewMatrix;
    }
    Transform::Transform(glm::mat4 &viewMatrix){
        this->viewMatrix = viewMatrix;
        this->modelMatrix = glm::mat4(1.0f);
        this->normalMatrix = glm::mat3(1.0f);
    }
    Transform::~Transform()
    {
    }

    void Transform::Transformation(glm::mat4 &viewMatrix){
        InverseTranspose(this->viewMatrix);
    }

    void Transform::Transformation(glm::vec3 &scaleVector,
                            glm::f32 angleInGrades, glm::vec3 &axis,
                            glm::vec3 &translationVector)
    {
        ResetToIdentity();


        Translate(translationVector);
        Rotate(angleInGrades, axis);
        Scale(scaleVector);

        InverseTranspose(this->viewMatrix);
    }

    void Transform::Transformation(glm::vec3 &scaleVector,
                            glm::f32 angleInGrades, glm::vec3 &axis,
                            glm::vec3 &translationVector,
                            glm::mat4 &viewMatrix)
    {
        ResetToIdentity();

        Translate(translationVector);
        Rotate(angleInGrades, axis);
        Scale(scaleVector);

        InverseTranspose(viewMatrix);
    }

    void Transform::Transformation()
    {
        ResetToIdentity();

        Translate(this->translation);
        Rotate(this->rotation.GetAngleDegree(), this->rotation.GetAxis());
        Scale(glm::vec3(this->scale));        

        InverseTranspose(this->viewMatrix);
    }


    Transform operator * (const Transform &b) const {
        Transform c;
        c.scale = scale * b.scale;
        c.rotation = b.rotation * rotation;
        c.translation = translation + rotation.Apply(b.translation * scale);

        return c;
    }

    Transform Inverse() const {
        Transform res;

        res.scale = 1/scale;
        res.rotation = rotation.inverse();
        res.translation = res.rotation.Apply(-translation * res.scale);
        return res;
    }

    void Transform::ResetToIdentity(){
        this->modelMatrix = glm::mat4(1.0f);
        this->normalMatrix = glm::mat3(1.0f);
    }  

    void Transform::Scale(glm::vec3 &scaleVector){
        this->modelMatrix = glm::scale(this->modelMatrix, scaleVector);
        this->scale = scaleVector.x;
    }
    void Transform::Scale(glm::f32 x, glm::f32 y, glm::f32 z){
        this->modelMatrix = glm::scale(this->modelMatrix, glm::vec3(x, y, z));
        this->scale = x;
    }

    void Transform::Rotate(glm::f32 angleInGrades, glm::vec3 &axis){
        this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(angleInGrades), axis);
        Quaternion q (axis, angleInGrades);
        this->rotation = q;
    }

    void Transform::Translate(glm::vec3 &translationVector){
        this->modelMatrix = glm::translate(this->modelMatrix, translationVector);
        this->translation = translationVector;
    }
    void Transform::Translate(glm::f32 x, glm::f32 y, glm::f32 z){
        this->modelMatrix = glm::translate(this->modelMatrix, glm::vec3(x, y, z));
        this->translation = glm::vec3(x, y, z);
    }

    void Transform::InverseTranspose(glm::mat4 &viewMatrix){
        this->normalMatrix = glm::inverseTranspose(glm::mat3(viewMatrix * this->modelMatrix));
    }

    glm::vec3 GetTranslationVector(){
        glm::vec3 translationVec = glm::vec3(
            modelMatrix[3].x,
            modelMatrix[3].y,
            modelMatrix[3].z
        );

        return translationVec;
    }

    glm::vec3 GetTranslationVectorWorld(glm::mat4 CumulatedModelMatrixToWorld){
        glm::vec3 translationVec = glm::vec3(
            modelMatrix[3].x,
            modelMatrix[3].y,
            modelMatrix[3].z
        );

        glm::vec3 outVec = glm::vec3(glm::vec4(translationVec, 1.0f) * CumulatedModelMatrixToWorld);

        return outVec;
    }

    glm::mat4 GetModelMatrix(){
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, translation);
        modelMatrix = glm::rotate(modelMatrix, rotation.GetAngleRad(), rotation.GetAxis());
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    }

    glm::mat4 GetNormalMatrix(){
        return glm::inverseTranspose(glm::mat3(viewMatrix * GetModelMatrix()));
    }

};