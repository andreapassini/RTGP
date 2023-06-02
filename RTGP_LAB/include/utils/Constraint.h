#pragma once

#include <utils/particle.h>

class Constraint
{
private:
	float rest_distance; // the length between particle p1 and p2 in rest configuration

public:
	Particle *p1, *p2; // the two particles that are connected through this constraint

	Constraint(Particle *p1, Particle *p2, float rest) :  p1(p1),p2(p2),rest_distance(rest)
	{
		glm::vec3 vec = p1->getPos()-p2->getPos();
		//rest_distance = glm::length(vec);
	}

	// Set K at like 2.0f
	void satisfyConstraint_Physics(float K)
	{
		glm::vec3 p1_to_p2 = this->p2->getPos() - this->p1->getPos(); // vector from p1 to p2
		float current_distance = glm::length(p1_to_p2); // current distance between p1 and p2
		p1_to_p2 /= current_distance;

		float delta = current_distance - rest_distance;

		glm::vec3 correctionVector = p1_to_p2 * delta * K; // The offset vector that could moves p1 into a distance of rest_distance to p2
		this->p1->addForce(correctionVector); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
		this->p2->addForce(-correctionVector); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	
	}

	// Set K between .5f - .25f
	void satisfyConstraint(float K)
	{
		glm::vec3 p1_to_p2 = this->p2->getPos() - this->p1->getPos(); // vector from p1 to p2
		float current_distance = glm::length(p1_to_p2); // current distance between p1 and p2
		p1_to_p2 /= current_distance;

		float delta = current_distance - rest_distance;

		glm::vec3 correctionVector = p1_to_p2 * delta * K; // The offset vector that could moves p1 into a distance of rest_distance to p2
		
		this->p1->offsetPos(correctionVector); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
		this->p2->offsetPos(-correctionVector); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	
	}
};
