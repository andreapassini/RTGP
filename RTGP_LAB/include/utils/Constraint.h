#pragma once

#include <utils/particle.h>

class Constraint
{
private:
	float rest_distance; // the length between particle p1 and p2 in rest configuration

public:
	Particle *p1, *p2; // the two particles that are connected through this constraint

	Constraint(Particle *p1, Particle *p2) :  p1(p1),p2(p2)
	{
		glm::vec3 vec = p1->getPos()-p2->getPos();
		rest_distance = vec.length();
	}

	/* This is one of the important methods, where a single constraint between two particles p1 and p2 is solved
	the method is called by Cloth.time_step() many times per frame*/
	void satisfyConstraint_Physics()
	{
		glm::vec3 p1_to_p2 = this->p2->getPos() - this->p1->getPos(); // vector from p1 to p2
		float current_distance = glm::length(p1_to_p2); // current distance between p1 and p2

		float delta = current_distance - rest_distance;
		glm::normalize(p1_to_p2);

		glm::vec3 correctionVector = p1_to_p2 * delta; // The offset vector that could moves p1 into a distance of rest_distance to p2
		float correctionRation = 0.5f;
		glm::vec3 correctionVectorHalf = glm::vec3(correctionVector.x, correctionVector.t, correctionVector.z);
		correctionVectorHalf *= correctionRation * K; // Lets make it half that length, so that we can move BOTH p1 and p2.
		this->p1->addForce(correctionVectorHalf); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
		this->p2->addForce(-correctionVectorHalf); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	
		std::cout << "Rest distance: " << rest_distance << ", Distance " << current_distance << std::endl;
		std::cout << "Corr 1: " << glm::length(correctionVectorHalf) << " - FORCE " << p1->force.x << ", " << p1->force.y << ", " << p1->force.z << ", " << std::endl;
		std::cout << "Corr 2: " << glm::length(-correctionVectorHalf) << " - FORCE " << p1->force.x << ", " << p1->force.y << ", " << p1->force.z << ", " << std::endl;
	}

	void satisfyConstraint()
	{
		glm::vec3 p1_to_p2 = p2->getPos()-p1->getPos(); // vector from p1 to p2
		float current_distance = p1_to_p2.length(); // current distance between p1 and p2

		float delta = current_distance - rest_distance;
		glm::normalize(p1_to_p2);

		glm::vec3 correctionVector = p1_to_p2 * delta; // The offset vector that could moves p1 into a distance of rest_distance to p2
		float correctionRation = 0.5f;

		glm::vec3 correctionVectorHalf = correctionVector * correctionRation * K; // Lets make it half that length, so that we can move BOTH p1 and p2.
		
		p1->offsetPos(correctionVectorHalf); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
		p2->offsetPos(-correctionVectorHalf); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	
	}
};
