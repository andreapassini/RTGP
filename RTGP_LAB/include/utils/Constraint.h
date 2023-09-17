#pragma once

#include <utils/particle.h>
#include <utils/particlesToCut.h>

class Constraint
{
private:
	float rest_distance; // the length between particle p1 and p2 in rest configuration
	float cuttingDistanceMultiplier;
public:
	Particle *p1, *p2; // the two particles that are connected through this constraint
	bool cuttable;

	Constraint(Particle *p1, Particle *p2, float rest, float cuttingMultiplier) :  p1(p1),p2(p2),rest_distance(rest),cuttable(false),cuttingDistanceMultiplier(cuttingMultiplier)
	{
	}

	void satisfyPositionalConstraint(float K)
	{
		glm::vec3 correctionVector = CalculateCorrectionVector(K);

		this->p1->offsetPos(correctionVector); 
		this->p2->offsetPos(-correctionVector);	
	}
	void satisfyPhysicsConstraint(float K)
	{
		glm::vec3 correctionVector = CalculateCorrectionVector(K);

		this->p1->addForce(correctionVector); 
		this->p2->addForce(-correctionVector);	
	}

	void satisfyAdvancedPhysicalConstraint(float K, float U, float deltaTime){
		glm::vec3 correctionVector = CalculateCorrectionVector(K);

		this->p1->addForce(correctionVector);
		this->p2->addForce(-correctionVector);

		glm::vec3 springFrictionVector = CalculateSpringFrictionVector( U, deltaTime);

		this->p1->addForce(springFrictionVector);
		this->p2->addForce(-springFrictionVector);
	}

private:
	glm::vec3 CalculateCorrectionVector(float K){
		glm::vec3 p1_to_p2 = this->p2->getPos() - this->p1->getPos(); // vector from p1 to p2
		float current_distance = glm::length(p1_to_p2); // current distance between p1 and p2

		if(cuttable){
			if(current_distance >= rest_distance * cuttingDistanceMultiplier){
				if(p1->movable)
					ParticlesToCut::GetInstance()->particles.push_back(p1);
				if(p2->movable)
					ParticlesToCut::GetInstance()->particles.push_back(p2);
			}
		}

		p1_to_p2 /= current_distance;

		float deltaDistance = current_distance - rest_distance;

		glm::vec3 correctionVector = K * deltaDistance * p1_to_p2;
		return correctionVector;
	}
	glm::vec3 CalculateSpringFrictionVector(float dampingFactor, float deltaTime){
		// dampingFactor(d*(v2-v1))*d
		glm::vec3 p1_to_p2 = this->p2->getPos() - this->p1->getPos(); // vector from p1 to p2
		float current_distance = glm::length(p1_to_p2); // current distance between p1 and p2

		p1_to_p2 /= current_distance;	// Normalize

		glm::vec3 v_1 = (this->p1->pos - this->p1->old_pos) / deltaTime; // Speed as the deltaDistance pos in time
		glm::vec3 v_2 = (this->p2->pos - this->p2->old_pos) / deltaTime; // Speed as the deltaDistance pos in time
		glm::vec3 v2_minus_v1 = v_2 - v_1; 

		glm::vec3 correctionVector = dampingFactor * (p1_to_p2 * (v2_minus_v1)) * p1_to_p2;
		return correctionVector;
	}
};

enum ConstraintType {
	POSITIONAL = 0,
	PHYSICAL,
	PHYSICAL_ADVANCED
};