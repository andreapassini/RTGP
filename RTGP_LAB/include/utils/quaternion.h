#pragma once

#include <cmath>
#include "../glm/glm.hpp"

class Quaternion
{
private:
public:
    glm::vec3 imaginary;
    float real;
    Quaternion(glm::vec3 axis, float angle){
        this->imaginary = axis * sin(angle*0.5f);
        this->real = cos(angle*0.5f);
    };
    ~Quaternion();

    void ComplexConjugate(){
        this->imaginary = -this->imaginary;
    }
    Quaternion ComplexConjugate(Quaternion &q){
        return Quaternion::ComplexConjugate(q);
    }
    static Quaternion ComplexConjugate(Quaternion &q){
        glm::vec3 invAxis = -q.imaginary;
        Quaternion q_conj = Quaternion(invAxis, q.real);
        return q_conj;
    }
    
    void Apply(Quaternion &p){
        *this = Quaternion::Apply(*this, p);
    };
    void Apply(glm::vec3 &postion){
        this->imaginary = Quaternion::Apply(*this, postion);
    };
    Quaternion Apply(Quaternion &q, Quaternion &p){
        Quaternion rotated = q*p*ComplexConjugate(q);
        return rotated;
    }
    glm::vec3 Apply(Quaternion &q, glm::vec3 &postion){
        Quaternion p = Quaternion(postion, 0.0f);
        Quaternion rotated = Apply(q, p);
        return rotated.imaginary;
    }
    
    Quaternion operator*(const Quaternion& q) const{
        glm::vec3 imaginary = imaginary*q.real + q.imaginary*real + glm::cross(imaginary, q.imaginary);
        float real = real*q.real - glm::dot(imaginary, q.imaginary);
        return Quaternion(imaginary, real);
    }
    Quaternion multiply(const Quaternion& q) const{
        glm::vec3 imaginary = imaginary*q.real + q.imaginary*real + glm::cross(imaginary, q.imaginary);
        float real = real*q.real - glm::dot(imaginary, q.imaginary);
        return Quaternion(imaginary, real);
    }
    void operator*=(const Quaternion& q){
        (*this)=multiply(q);
    }

    void operator*=(const float value){
        imaginary*=value;
        real*=value;
    }
    Quaternion operator*(const float value)const{
        float scalar=real*value;
        glm::vec3 imaginary=imaginary*value;

        return Quaternion(imaginary, scalar);
    }

    float magnitude(const Quaternion& q){
        float mag = 0.0f;
        mag += glm::length(q.imaginary);
        mag += q.real;
        return mag;
    }
    Quaternion normalize(const Quaternion& q){
        float magnitude = Quaternion::magnitude(q);
        Quaternion p = Quaternion(glm::vec3(0.0f), 0.0f);
        p.imaginary = q.imaginary / magnitude;
        p.real /= magnitude;
        return p;
    }
    void normalize(Quaternion& q){
        float magnitude = Quaternion::magnitude(q);
        q.imaginary = q.imaginary / magnitude;
        q.real /= magnitude;
    }
};
