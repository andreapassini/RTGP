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

		this->p1->offsetPos(correctionVector); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
		this->p2->offsetPos(-correctionVector); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	
	}
	void satisfyPhysicsConstraint(float K)
	{
		glm::vec3 correctionVector = CalculateCorrectionVector(K);

		this->p1->addForce(correctionVector); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
		this->p2->addForce(-correctionVector); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	
	}

	void satisfyAdvancedPhysicalConstraint(float K, float U, float deltaTime){
		glm::vec3 correctionVector = CalculateCorrectionVector(K);

		this->p1->addForce(correctionVector); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
		this->p2->addForce(-correctionVector); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	

		glm::vec3 springFrictionVector = CalculateSpringFrictionVector( U, deltaTime);

		this->p1->addForce(springFrictionVector); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
		this->p2->addForce(-springFrictionVector); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	

	}

private:
	glm::vec3 CalculateCorrectionVector(float K){
		glm::vec3 p1_to_p2 = this->p2->getPos() - this->p1->getPos(); // vector from p1 to p2
		float current_distance = glm::length(p1_to_p2); // current distance between p1 and p2
		float abs_cur = abs(current_distance);

		// if(abs_cur <= 2 *FLT_EPSILON){
		// 	if(current_distance >= 0.0f)
		// 		current_distance += 2 * FLT_EPSILON;
		// 	else 
		// 		current_distance -= 2 * FLT_EPSILON;
		// }

		if(cuttable){
			if(current_distance >= rest_distance * cuttingDistanceMultiplier){
				ParticlesToCut::GetInstance()->particles.push_back(p1);
				ParticlesToCut::GetInstance()->particles.push_back(p2);
			}
		}


		p1_to_p2 /= current_distance;

		float deltaDistance = current_distance - rest_distance;

		glm::vec3 correctionVector = K * deltaDistance * p1_to_p2; // The offset vector that could moves p1 into a distance of rest_distance to p2
		return correctionVector;
	}
	glm::vec3 CalculateSpringFrictionVector(float dampingFactor, float deltaTime){
		// (k * ((mag - l)/l) + u (v2 -v1) * n
		// u friction coeff
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