#pragma once

#define _USE_MATH_DEFINES

#include <cmath>
#include "../glm/glm.hpp"

class Quaternion
{
private:
public:
    glm::vec3 imaginary;
    float real;
    Quaternion(glm::vec3 axis, float angle){
        float angleRad = angle*(M_PI)/180;
        this->imaginary = axis * sin(angleRad*0.5f);
        this->real = cos(angleRad*0.5f);
    };
    ~Quaternion();

    void ComplexConjugate(){ this->imaginary = -this->imaginary; }
    Quaternion ComplexConjugate() const { return Quaternion(-imaginary, +real); }
    static const Quaternion ComplexConjugate(Quaternion q) { return Quaternion(-q.imaginary, +q.real); }

    Quaternion operator-() const { return Quaternion( -imaginary, -real ); }
    
    // arithmetic operators "OUT OF PLACE"
    Quaternion operator / (float d) const { return Quaternion(imaginary/d,real/d); }
    Quaternion operator * (float d) const { return Quaternion(imaginary*d,real*d); }
    Quaternion operator * (const Quaternion& q) const{
        glm::vec3 imaginary = imaginary*q.real + q.imaginary*real + glm::cross(imaginary, q.imaginary);
        float real = real*q.real - glm::dot(imaginary, q.imaginary);
        return Quaternion(imaginary, real);
    }
    Quaternion Multiply(const Quaternion& q) const{
        glm::vec3 imaginary = imaginary*q.real + q.imaginary*real + glm::cross(imaginary, q.imaginary);
        float real = real*q.real - glm::dot(imaginary, q.imaginary);
        return Quaternion(imaginary, real);
    }
    
    // arithmetic operators "IN PLACE"
    void operator /= (float d){ imaginary/=d; real/=d; }
    void operator *= (float d){ imaginary*=d; real*=d; }
    void operator *= (const Quaternion& q){ (*this)=Multiply(q); }
    void operator += (const Quaternion &q){ imaginary+=q.imaginary; real+=q.real; }
    void operator -= (const Quaternion &q){ imaginary-=q.imaginary; real-=q.real; }

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
    inline float deg2rad(float k){
        return k*(M_PI)/180;
    }
};
