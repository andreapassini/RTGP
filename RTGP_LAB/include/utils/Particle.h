#pragma once

#include <glm/glm.hpp>
#include <physicsSimulation/physicsSimulation.h>

#include <utils/Scene.h>
#include <colliders/PlaneCollider.h>
#include <colliders/sphereCollider.h>
#include <colliders/CapsuleCollider.h>

/* Some physics constants */
#define DAMPING 0.02f // how much to damp the cloth simulation each frame
#define TIME_STEPSIZE2 (0.5f*0.5f)
#define COLLISION_OFFSET_MULTIPLIER 1.15f

#define FIXED_TIME_STEP (1.0f / 60.0f)
#define FIXED_TIME_STEP2 (FIXED_TIME_STEP * FIXED_TIME_STEP)


/* The particle class represents a particle of mass that can move around in 3D space*/
class Particle
{
private:

public:
	glm::vec3 pos; 
	glm::vec3 normal;
	glm::vec3 old_pos;
	glm::vec3 force;
	bool movable; 
	float mass;

	glm::vec3 shader_force;
	bool renderable;
	
	Particle(glm::vec3 pos, float m) : pos(pos), normal(glm::vec3(1.0f)),  old_pos(pos),force(glm::vec3(0.0f)), mass(m), movable(true){
		renderable = true;
	}
	Particle(){}

	void addForce(glm::vec3 f)
	{
		this->force += f;
		//\\shader_force += f;
	}

	void PhysicStep()
	{
		if(!movable){
			this->force = glm::vec3(0.0f);
			return;
		}

		glm::vec3 now_pos = pos;
		glm::vec3 accel = this->force/mass;
		//pos = now_pos + (now_pos-old_pos) * (1.0f-DAMPING) + accel * FIXED_TIME_STEP2;	
		//pos = ((2.0f) * now_pos) - ( old_pos) + (accel * FIXED_TIME_STEP2);
		pos = ((2.0f - DAMPING) * now_pos) - ( (1.0f - DAMPING) * old_pos) + (accel * FIXED_TIME_STEP2);
		old_pos = now_pos;
		this->shader_force = glm::vec3(force.x, force.y, force.z) + glm::vec3(0.1f);
		this->force = glm::vec3(0.0f);
	}
	void PhysicStep(float deltaTime)
	{
		if(!movable){
			this->force = glm::vec3(0.0f);
			return;	
		}

		glm::vec3 now_pos = pos;
		glm::vec3 accel = force/mass;
		pos = now_pos + ((now_pos-old_pos) * (1.0f-DAMPING) + accel) * (deltaTime) * TIME_STEPSIZE2;	
		//pos = now_pos + (now_pos-old_pos) * (1.0f-DAMPING) + accel * deltaTime * FIXED_TIME_STEP2;	
		old_pos = now_pos;
		this->shader_force = glm::vec3(force.x, force.y, force.z);
		this->force = glm::vec3(0.0f);
	}

	glm::vec3& getPos() {return pos;}

	void resetForce() {this->force = glm::vec3(0.0f);}

	void offsetPos(glm::vec3 v) 
	{
		if(movable) {
		 	pos += v;
			shader_force += v;
		}
	}

	void makeUnmovable() {movable = false;}

	void addToNormal(glm::vec3 n)
	{
		this->normal += n;	//glm::normalize(normal);
		glm::normalize(this->normal);
	}

	glm::vec3& getNormal() { return normal;} // notice, the normal is not unit length

	void resetNormal() {
		this->normal = glm::normalize(glm::vec3(1.0f));
	}

	void SphereCollision(glm::mat4 clothModelMatrix, const glm::vec3 centerWorld,const float radius){
		glm::vec3 v = glm::vec3(glm::vec4(this->getPos(), 1.0f) * clothModelMatrix) - centerWorld;

		float l = glm::length(v);

		if (l < (radius * COLLISION_OFFSET_MULTIPLIER)) // if the particle is inside the ball
		{
			this->offsetPos(glm::normalize(v) * (((radius * COLLISION_OFFSET_MULTIPLIER)-l))); // project the particle to the surface of the ball
		}
	}
	void SphereCollision(const glm::vec3 centerWorld,const float radius){
		glm::vec3 v = pos - centerWorld;

		float l = glm::length(v);

		if (l < (radius * COLLISION_OFFSET_MULTIPLIER)) // if the particle is inside the ball
		{
			this->offsetPos(glm::normalize(v) * (((radius * COLLISION_OFFSET_MULTIPLIER)-l))); // project the particle to the surface of the ball
		}
	}
	void SphereCollision(SphereCollider* SphereCollider){
		SphereCollision(SphereCollider->transform->GetTranslationVector(), SphereCollider->radius);
	}

	void PlaneCollision(const float yLimit){
		if (this->pos.y < yLimit)
		{
			float l = yLimit - this->pos.y;
			this->offsetPos(glm::vec3(0.0f, l, 0.0f));
		}
	}
	void PlaneCollision(const glm::vec3 normal, const glm::vec3 pointOnPlane){
		glm::vec3 distance = this->pos - pointOnPlane;
		float dot = glm::dot(distance, normal);

		if(dot <= 0.0f){
			// under the plane
			glm::vec3 reposition = normal * (-1 * dot * COLLISION_OFFSET_MULTIPLIER);
			this->offsetPos(reposition);
		}
	}
	void PlaneCollision(PlaneCollider* planeCollider){
		PlaneCollision(planeCollider->normal, planeCollider->transform->GetTranslationVector());
	}

	void CapsuleCollision(glm::mat4 clothModelMatrix, glm::vec3 p1, glm::vec3 p2, float radius){
		glm::vec3 posWorld = glm::vec3(glm::vec4(this->getPos(), 1.0f) * clothModelMatrix);
		glm::vec3 segment = p2 - p1;
		float segmentMagnitude = glm::length(segment);

		glm::vec3 p1_p = posWorld - p1;
		glm::vec3 p2_p = posWorld - p2;
		
		float p1_p_distance = glm::length(p1_p);
		float p2_p_distance = glm::length(p2_p);

		if(p1_p_distance > radius && p2_p_distance > radius ){
			return;
		}

		// Just like a sphere
		float p1_p_dot_segment = glm::dot(p1_p, segment);
		if(p1_p_dot_segment >= segmentMagnitude){
			SphereCollision(clothModelMatrix, p2, radius);
		} else if (p1_p_dot_segment <= 0.0f){
			SphereCollision(clothModelMatrix, p1, radius);
		}

		// distance with segment and angle 90
		glm::vec3 p = (segment / segmentMagnitude) * p1_p_dot_segment;
		glm::vec3 distance = p - p1_p_dot_segment;
		float distanceMagnitude = glm::length(distance);

		if(distanceMagnitude < (radius * COLLISION_OFFSET_MULTIPLIER)){
			// relocate as sphere
			this->offsetPos((distance / distanceMagnitude) * ((radius * COLLISION_OFFSET_MULTIPLIER) - distanceMagnitude)); // project the particle to the surface of the ball
		}

	}
	void 
	CapsuleCollision(glm::vec3 p1, glm::vec3 p2, float radius){
		glm::vec3 posWorld = pos;
		glm::vec3 segment = p2 - p1;
		float segmentMagnitude = glm::length(segment);

		glm::vec3 p1_p = posWorld - p1;
		glm::vec3 p2_p = posWorld - p2;
		
		float p1_p_distance = glm::length(p1_p);
		float p2_p_distance = glm::length(p2_p);

		// Just like a sphere
		float p1_p_dot_segment = glm::dot(p1_p, segment);
		float p2_p_dot_segment = glm::dot(p2_p, -segment);
		if(p1_p_dot_segment < 0.0f){
			SphereCollision(p1, radius);
		} else if (p2_p_dot_segment < 0.0f){
			SphereCollision(p2, radius);
		}

		// distance with segment and angle 90
		glm::vec3 p = (segment / segmentMagnitude) * p1_p_dot_segment;
		glm::vec3 distance = p - p1_p_dot_segment;
		float distanceMagnitude = glm::length(distance);

		if(distanceMagnitude < (radius * COLLISION_OFFSET_MULTIPLIER)){
			// relocate as sphere
			this->offsetPos((distance / distanceMagnitude) * ((radius * COLLISION_OFFSET_MULTIPLIER) - distanceMagnitude)); // project the particle to the surface of the ball
		}

	}
	void CapsuleCollision(CapsuleCollider* capsuleCollider){
		CapsuleCollision(capsuleCollider->p1->GetTranslationVector(), capsuleCollider->p2->GetTranslationVector(), capsuleCollider->radius);
	}



	void CubeCollision(glm::mat4 clothModelMatrix, const glm::vec3 cubeCenterInWorldSpace, const float edge){
		float halfEdge = edge * 0.5f;
		glm::vec3 myPosWorld = glm::vec3(glm::vec4(this->getPos(), 1.0f) * clothModelMatrix);
		glm::vec3 distanceVector = myPosWorld - cubeCenterInWorldSpace;
		/*
		                      Y
                              |
                              |
                              |________X
                             /
                            /
                           /
                          Z
		*/

		// DETECTION
		// Do the Test on the 3 different coordinates
		if((myPosWorld.x > (cubeCenterInWorldSpace.x - halfEdge) && myPosWorld.x < (cubeCenterInWorldSpace.x + halfEdge)) &&
			(myPosWorld.y > (cubeCenterInWorldSpace.y - halfEdge) && myPosWorld.y < (cubeCenterInWorldSpace.y + halfEdge)) &&
			(myPosWorld.z > (cubeCenterInWorldSpace.z - halfEdge) && myPosWorld.z < (cubeCenterInWorldSpace.z + halfEdge)))
		{
			// I'm inside the cube
			// REACTION
			// Check the max diff between pos and cubeCenter and displace
			if(abs(distanceVector.x) > abs(distanceVector.y) && abs(distanceVector.x) > abs(distanceVector.z)){
				// Displace on X
				float displacement = glm::dot(distanceVector, glm::vec3(1.0f, 0.0f, 0.0f));
				this->pos = glm::vec3(this->pos.x + displacement, this->pos.y, this->pos.z);
			}
			else if(abs(distanceVector.y) > abs(distanceVector.x) && abs(distanceVector.y) > abs(distanceVector.z)){
				// Displace on Y
				float displacement = glm::dot(distanceVector, glm::vec3(0.0f, 1.0f, 0.0f));
				this->pos = glm::vec3(this->pos.x, this->pos.y + displacement, this->pos.z);
			} 
			else{
				// Displace on Z
				float displacement = glm::dot(distanceVector, glm::vec3(0.0f, 1.0f, 0.0f));
				this->pos = glm::vec3(this->pos.x, this->pos.y, this->pos.z + displacement);
			}
		}
	};
	bool operator== (Particle &p){
		bool equal = false;

		if(this->pos == p.pos && this->normal == p.normal)
			equal = true;

		return equal;
	};
};
