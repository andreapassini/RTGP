#pragma once

#include <glm/glm.hpp>

/* Some physics constants */
#define DAMPING 0.01f // how much to damp the cloth simulation each frame
#define TIME_STEPSIZE2 0.5f*0.5f // how large time step each particle takes each frame
#define SPHERE_OFFSET_MULTIPLIER 1.25f

/* The particle class represents a particle of mass that can move around in 3D space*/
class Particle
{
private:

public:
	glm::vec3 pos; // the current position of the particle in 3D space
	glm::vec3 normal; // an accumulated normal (i.e. non normalized), used for OpenGL soft shading
	glm::vec3 old_pos; // the position of the particle in the previous time step, used as part of the verlet numerical integration scheme
	glm::vec3 force; // a vector representing the current force of the particle
	bool movable; // can the particle move or not ? used to pin parts of the cloth
	float mass; // the mass of the particle (is always 1 in this example)

	bool renderable;
	
	Particle(glm::vec3 pos) : pos(pos), normal(glm::vec3(1.0f)),  old_pos(pos),force(glm::vec3(0.0f)), mass(1.0f), movable(true){
		renderable = true;
	}
	Particle(){}

	void addForce(glm::vec3 f)
	{
		this->force += f;
	}

	void PhysicStep(float deltaTime)
	{
		if(!movable){
			this->force = glm::vec3(0.0f);
			return;
		}

		glm::vec3 now_pos = pos;
		glm::vec3 accel = force/mass;
		pos = now_pos + ((now_pos-old_pos) * (1.0f-DAMPING) + accel) * deltaTime * TIME_STEPSIZE2;	// newPos = now_pos + speed * deltaTime
		old_pos = now_pos;
		this->force = glm::vec3(0.0f);
	}

	glm::vec3& getPos() {return pos;}

	void resetForce() {this->force = glm::vec3(0.0f);}

	void offsetPos(glm::vec3 v) 
	{
		if(movable) 
		 	pos += v;
	}

	void makeUnmovable() {movable = false;}

	void addToNormal(glm::vec3 n)
	{
		this->normal += n;	//glm::normalize(normal);
	}

	glm::vec3& getNormal() { return normal;} // notice, the normal is not unit length

	void resetNormal() {this->normal = glm::vec3(0.0f);}

	void BallCollision(glm::mat4 matrixToWorld, const glm::vec3 centerWorld,const float radius){
		glm::vec3 v = glm::vec3(glm::vec4(this->getPos(), 1.0f) * matrixToWorld) - centerWorld;
		float l = glm::length(v);
		if (glm::length(v) < radius) // if the particle is inside the ball
		{
			this->offsetPos(glm::normalize(v) * ((radius-l) * SPHERE_OFFSET_MULTIPLIER)); // project the particle to the surface of the ball
		}
	}
	void PlaneCollision(const float yLimit){
		if (this->pos.y < yLimit) // if the particle is inside the ball
		{
			this->pos = glm::vec3(this->pos.x, yLimit, this->pos.z); // project the particle to the surface of the ball
		}
	}

	bool operator== (Particle &p){
		bool equal = false;

		if(this->pos == p.pos && this->normal == p.normal)
			equal = true;

		return equal;
	};
};
