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
        float angleRad = glm::radians(angle);
        this->imaginary = axis * sin(angleRad*0.5f);
        this->real = cos(angleRad*0.5f);
    };
    // empty construction: returns quaternion 1 + 0 * i
    Quaternion():Quaternion(glm::vec3(0.0f), 1){}

    ~Quaternion(){
    }

    void ComplexConjugate(){ this->imaginary = -this->imaginary; }
    Quaternion ComplexConjugate() const { return Quaternion(-imaginary, +real); }
    static const Quaternion ComplexConjugate(Quaternion q) { return Quaternion(-q.imaginary, +q.real); }

    Quaternion operator-() const { return Quaternion( -imaginary, -real ); }
    
    // arithmetic operators "OUT OF PLACE"
    Quaternion operator = (const Quaternion& q) const { return Quaternion(q.imaginary, q.real); }
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

    glm::vec3 Apply(const Quaternion &q, const glm::vec3 &postion) const {
        Quaternion p = Quaternion(postion, 0.0f);
        Quaternion rotated = ComplexConjugate(q) * p * q;
        return rotated.imaginary;
    }
    glm::vec3 Apply(const glm::vec3 &postion) const {
        return Apply((*this), postion);
    }
    
    void invert() {
        ComplexConjugate();
        operator /= ( squaredMagnitude() );
    }
    Quaternion inverse() const{
        return ComplexConjugate() / squaredMagnitude();
    }

    float squaredMagnitude() const {
        float mag = (imaginary.x*imaginary.x) + (imaginary.y*imaginary.y) + (imaginary.z*imaginary.z) + (real*real);
        return mag;
    }
    float squaredMagnitude(const Quaternion& q){
        float mag = (q.imaginary.x*q.imaginary.x) + (q.imaginary.y*q.imaginary.y) + (q.imaginary.z*q.imaginary.z) + (q.real*q.real);
        return mag;
    }
    float magnitude(const Quaternion& q){
        float mag = sqrt(squaredMagnitude(q));
        return mag;
    }

    Quaternion normalize(const Quaternion& q){
        q = q / Quaternion::magnitude(q);;
        return q;
    }
    void normalize(){
        *this = Quaternion::normalize(*this);
    }

    float GetAngleRad(){
        return 2 * acos(real);
    }

    float GetAngleDegree(){
        return glm::degrees(2 * acos(real));
    }

    glm::vec3 GetAxis(){
        float angle = GetAngleRad();
        glm::vec3 axis = this->imaginary / sin(angle/2);
    }
};
