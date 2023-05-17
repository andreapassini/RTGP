#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glfw/glfw3.h>

enum Camera_Movement{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};


// Considering only yaw and pitch
const GLfloat YAW = -90.0f;
const GLfloat PITCH = -90.0f;

const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVITY = 0.25f;

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 WorldFront;
    glm::vec3 Up;
    glm::vec3 WorldUp;
    glm::vec3 Right;
    GLboolean onGround;
    GLfloat Yaw;
    GLfloat Pitch;
    GLfloat MovementSpeed;
    GLfloat MouseSensitivity;

    Camera(glm::vec3 position, GLboolean onGround);
    ~Camera();

    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime);
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch);
private:
    void updateCameraVectors();
};

Camera::Camera(glm::vec3 position, GLboolean onGround):
Position(position), onGround(onGround), Yaw(YAW), Pitch(PITCH), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
{  
    this->WorldUp = glm::vec3(0.0, 1.0, 0.0);
    this->updateCameraVectors();
}

Camera::~Camera()
{
}

glm::mat4 Camera::GetViewMatrix(){
    return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
}

void Camera::updateCameraVectors(){
    glm::vec3 front;
    front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
    front.y = sin(glm::radians(this->Pitch));
    front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));

    this->Front = this->WorldFront = glm::normalize(front);

    // I fwe are on ground, we want to keep moving on the plane
    this->WorldFront.y = 0.0f;
    this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
    this->Up = glm::normalize(glm::cross(this->Right, this->Front));
}

void Camera::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime){
    GLfloat velocity = this->MovementSpeed * deltaTime;
    if(direction == FORWARD){
        this->Position += (this->onGround ? this->WorldFront : this->Front) * velocity;
    }
    if(direction == BACKWARD){
        this->Position -= (this->onGround ? this->WorldFront : this->Front) * velocity;
    }
    if(direction == LEFT){
        this->Position -= this->Right * velocity;
    }
    if(direction == RIGHT){
        this->Position += this->Right * velocity;
    }
}

void Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = GL_TRUE){
    xoffset *= this->MouseSensitivity;
    yoffset *= this->MouseSensitivity;

    this->Yaw += xoffset;
    this->Pitch += yoffset;

    // No gimab lock cause only rot on y and x
    // But if we dont want to go back, in a full circle
    if(constraintPitch){
        if(this->Pitch > 89.0f){
            this->Pitch = 89.0f;
        } 
        if(this->Pitch < -89.0f){
            this->Pitch = 89.0f;
        }
    }

    this->updateCameraVectors();
}