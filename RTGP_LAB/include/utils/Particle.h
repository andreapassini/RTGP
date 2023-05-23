#pragma once

#include <glm/glm.hpp>

/* Some physics constants */
#define DAMPING 0.01f // how much to damp the cloth simulation each frame
#define TIME_STEPSIZE2 0.5f*0.5f // how large time step each particle takes each frame
#define CONSTRAINT_ITERATIONS 1 // how many iterations of constraint satisfaction each frame (more is rigid, less is soft)

/* The particle class represents a particle of mass that can move around in 3D space*/
class Particle
{
private:

public:
	glm::vec3 pos; // the current position of the particle in 3D space
	glm::vec3 normal; // an accumulated normal (i.e. non normalized), used for OpenGL soft shading
	bool movable; // can the particle move or not ? used to pin parts of the cloth
	float mass; // the mass of the particle (is always 1 in this example)
	glm::vec3 old_pos; // the position of the particle in the previous time step, used as part of the verlet numerical integration scheme
	glm::vec3 acceleration; // a vector representing the current acceleration of the particle
	
	Particle(glm::vec3 pos) : pos(pos), old_pos(pos),acceleration(glm::vec3(0.0f)), mass(1), movable(true), normal(glm::vec3(1.0f)){}
	Particle(){}

	void addForce(glm::vec3 f)
	{
		acceleration += f/mass;
	}

	/* This is one of the important methods, where the time is progressed a single step size (TIME_STEPSIZE)
	   The method is called by Cloth.time_step()
	   Given the equation "force = mass * acceleration" the next position is found through verlet integration*/
	void PhysicStep()
	{
		if(!movable)
			return;

		glm::vec3 temp = pos;
		pos = pos + (pos-old_pos)*(1.0f-DAMPING) + acceleration*TIME_STEPSIZE2;
		old_pos = temp;
		acceleration = glm::vec3(0.0f); // acceleration is reset since it HAS been translated into a change in position (and implicitly into velocity)	
	}

	glm::vec3& getPos() {return pos;}

	void resetAcceleration() {acceleration = glm::vec3(0.0f);}

	void offsetPos(const glm::vec3 v) { if(movable) pos += v;}

	void makeUnmovable() {movable = false;}

	void addToNormal(glm::vec3 normal)
	{
		normal += glm::normalize(normal); // normal.normalized();
	}

	glm::vec3& getNormal() { return normal;} // notice, the normal is not unit length

	void resetNormal() {normal = glm::vec3(0.0f);}

	void BallCollision(const glm::vec3 center,const float radius){
		glm::vec3 v = this->getPos()-center;
		float l = v.length();
		if ( v.length() < radius) // if the particle is inside the ball
		{
			this->offsetPos(glm::normalize(v)*(radius-l)); // project the particle to the surface of the ball
		}
	}

};
