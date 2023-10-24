#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>
#include <glm/common.hpp>
#include <iostream>
#include <math.h>       /* atan2 */

struct ModelAndNormalMatrix {
    glm::mat4 modelMatrix;
    glm::mat3 normalMatrix;
};

const float GetAngleDegree(glm::quat quaternion){
    glm::vec3 axis = glm::vec3(0.0f);
    
    glm::vec3 axAndSin = glm::vec3(quaternion.x, quaternion.y, quaternion.z);

    float sinHalfA = 0.0f;
    float cosHalfA = 0.0f;

    sinHalfA = glm::length(axAndSin);
    cosHalfA = quaternion.w;

    float angle = 2 * atan2(sinHalfA, cosHalfA);
    return angle;
}

const glm::vec3 GetAxis(glm::quat quaternion){
    glm::vec3 axis = glm::vec3(0.0f);
    
    glm::vec3 axAndSin = glm::vec3(quaternion.x, quaternion.y, quaternion.z);
    axis = glm::normalize(axAndSin);

    return axis;
}

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
    glm::vec3 translation;
    glm::quat rotation;
    float scale;

    Transform(){        
        this->scale = 1.0f;
        this->rotation = glm::quat();
        this->translation = glm::vec3(0.0f);
    }
    ~Transform()
    {
    }

    ModelAndNormalMatrix Transformation(glm::mat4 &viewMatrix)
    {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::mat3 normalMatrix = glm::mat3(1.0f);

        glm::vec3 scaleVector = glm::vec3(this->scale);
        modelMatrix = glm::scale(modelMatrix, scaleVector);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(GetAngleDegree(this->rotation)), GetAxis(this->rotation));
        modelMatrix = glm::translate(modelMatrix, this->translation);

        normalMatrix = glm::inverseTranspose(glm::mat3(viewMatrix * modelMatrix));

        ModelAndNormalMatrix out;
        out.modelMatrix = modelMatrix;
        out.normalMatrix = normalMatrix;

        return out;
    }

    Transform operator * (const Transform &b) const {
        Transform c;
        c.scale = scale * b.scale;
        c.rotation = (b.rotation) * rotation;
        c.translation = (this->translation) + glm::rotate(this->rotation, (b.translation)* scale);

        return c;
    }

    Transform Inverse() const {
        Transform res;

        res.scale = 1/scale;
        res.rotation = glm::inverse(rotation);
        (res.translation) = glm::rotate(res.rotation, -(translation) * res.scale);
        return res;
    }

    glm::mat4 GetModelMatrix(){
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, (translation));
        modelMatrix = glm::rotate(modelMatrix, GetAngleDegree(*this->rotation), GetAxis(this->rotation));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    }

    glm::mat4 GetNormalMatrix(const glm::mat4 &viewMatrix){
        return glm::inverseTranspose(glm::mat3(viewMatrix * GetModelMatrix()));
    }

    glm::vec3 Apply(const glm::vec3& p) const{
        return glm::rotate(this->rotation, (p * this->scale) + (this->translation));
    }

    void PrintTransform(){
        std::cout << "Transform: "<< std::endl;
        std::cout << "  - Translation: " << (this->translation).x 
            << " " << (this->translation).y 
            << " " << (this->translation).z 
            << std::endl;
        std::cout << "  - Rotation: " << 
            GetAxis(this->rotation).x << " " << GetAxis(this->rotation).y << " "
            << GetAxis(this->rotation).z << " "
            << GetAngleDegree(this->rotation) << std::endl;
        std::cout << "  - Scale: " << 
            this->scale << std::endl;
    }
};