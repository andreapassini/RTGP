#pragma once

#include <utils/constraint.h>
#include <vector>
#include <glad/glad.h>
#include <physicsSimulation/physicsSimulation.h>

// GLFW
#include <glfw/glfw3.h>
#include <utils/Transform.h>

#include <cstdlib>
#include <random>
#include <ctime>
#include <iostream>

#define FIXED_TIME_STEP (1.0f / 60.0f)
#define FIXED_TIME_STEP2 (FIXED_TIME_STEP * FIXED_TIME_STEP)


class Cloth
{
private:
	Transform *transform;
	int dim; // number of particles in "width" direction
	// total number of particles is dim*dim

	std::vector<Constraint> constraints; // alle constraints between particles as part of this cloth
	ConstraintType springsType;
	unsigned int constraintIterations;
	unsigned int collisionIterations;
	float gravityForce;

	GLuint VAO;
	GLuint EBO;
	GLuint VBO;
    std::vector<GLuint> indices;

	float maxForce;

	Particle* getParticle(int x, int y, int rowDim) {return &particles[x*rowDim + y];}
	void makeConstraint(Particle *p1, Particle *p2, float rest_distance) {
		constraints.push_back(Constraint(p1,p2, rest_distance));
	}

	glm::vec3 CalculateNormalSquare(float x, float y)
	{
		if(x < dim - 1 ||
			y < dim -1)
			return glm::normalize(glm::vec3(1.0f));

		float idTopLeft = (x * dim) + y;

		/*
			COUNTER CLOCK-WISE
			y
		x	1---4	
			|	|
			|	|	
			2---3
			
	(x,y)   *--* (x,y+1)
	        | /|
	        |/ |
	(x+1,y) *--* (x+1,y+1)
		
		*/

		glm::vec3 pos1 = getParticle( x     , y		 , dim)->pos;
		glm::vec3 pos2 = getParticle( x		, (y + 1), dim)->pos;
		glm::vec3 pos3 = getParticle((x + 1), y		 , dim)->pos;
		glm::vec3 pos4 = getParticle((x + 1), (y + 1), dim)->pos;

		/*
		   ^
			\
			 \
		*/
		glm::vec3 diagonal1 = pos1-pos4;
		
		/*
			  ^				  
			 /
			/
		*/
		glm::vec3 diagonal2 = pos2-pos3;

		return glm::cross(diagonal2, diagonal1);
	}
	void SetUp()
	{
		// we create the buffers
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);
        
		glBindVertexArray(this->VAO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		// Create triangles from grid
		MakeTriangleFromGrid();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), this->indices.data(), GL_STATIC_DRAW);
		UpdateNormals();
		// we copy data in the VBO - we must set the data dimension, and the pointer to the structure containing the data
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->particles.size() * sizeof(Particle), &this->particles[0], GL_DYNAMIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, pos));
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, normal));
	
		glEnableVertexAttribArray(2);
		if(springsType == PHYSICAL || springsType == PHYSICAL_ADVANCED){
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, force));
		} else {
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, shader_force));
		}

		// Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
        glBindVertexArray(0); 
	}
	void MakeTriangleFromGrid(){
		indices.clear();
		for(int x = 0; x < dim-1; x++)
		{
			for(int y=0; y < dim-1; y++)
			{
				if(getParticle(x, y, dim)->renderable && 
					getParticle(x, y+1, dim)->renderable &&
					getParticle(x+1, y, dim)->renderable)
				{
					indices.push_back((x)*dim +y);
					indices.push_back((x)*dim +(y+1));
					indices.push_back((x+1)*dim +(y));
				}

				if(getParticle(x+1, y, dim)->renderable && 
					getParticle(x, y+1, dim)->renderable &&
					getParticle(x+1, y+1, dim)->renderable)
				{
					indices.push_back((x+1)*dim +(y));
					indices.push_back((x)*dim +(y+1));
					indices.push_back((x+1)*dim +(y+1));
				}
			}
		}
	}
	void UpdateNormals(){
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->resetNormal();
		}

		for(int x = 0; x < dim-1; x++)
		{
			for(int y=0; y < dim-1; y++)
			{
				glm::vec3 normal = CalculateNormalSquare(x, y);
				getParticle(x,   y,   dim)->addToNormal(normal);
				getParticle(x+1, y,   dim)->addToNormal(normal);
				getParticle(x,   y+1, dim)->addToNormal(normal);
				getParticle(x+1, y+1, dim)->addToNormal(normal);
			}
		}

		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			if(particle->normal.x == 0.0f && particle->normal.y == 0.0f && particle->normal.z == 0.0f)
				return;

			particle->normal = glm::normalize(particle->normal);
		}	
	}
	void UpdateBuffers(){
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->dim * this->dim * sizeof(Particle), this->particles.data(), GL_DYNAMIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, pos));
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, normal));

		glEnableVertexAttribArray(2);
		if(springsType == PHYSICAL || springsType == PHYSICAL_ADVANCED){
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, force));
		} else {
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, shader_force));
		}

		glBindVertexArray(0);
	}
	void freeGPUresources()
    {
        // If VAO is 0, this instance of Mesh has been through a move, and no longer owns GPU resources,
        // so there's no need for deleting.
        if (VAO)
        {
            glDeleteVertexArrays(1, &this->VAO);
            glDeleteBuffers(1, &this->VBO);
            glDeleteBuffers(1, &this->EBO);
        }
    }

	int FindIndexParticle(Particle p){
		for(size_t i = 0; i <= particles.size(); i++ )
		{							
			if(particles[i] == p)
				return i;
		}
	}
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	std::vector<Particle> particles; // all particles that are part of this cloth
	float K;
	float U;

	Cloth(int dim, float particleDistance, glm::vec3 topLeftPosition, Transform *t, bool pinned, ConstraintType usePhysicConstraints, float k, float u, unsigned int contraintIt, float gravity, float m, unsigned int collisionIt){
		this->dim = dim;
		this->transform = t;
		this->springsType = usePhysicConstraints;
		this->constraintIterations = contraintIt;
		this->collisionIterations = collisionIt;
		this->gravityForce = gravity;
		this->K = k;
		this->U = u;

		maxForce = 0.0f;
		particles.resize(dim*dim); //I am essentially using this vector as an array with room for num_particles_width*dim particles
		
		// creating particles in a grid of particles from (0,0,0) to (width,-height,0)
		for(int x=0; x < dim; x++)
		{
			for(int y=0; y < dim; y++)
			{
				// To keep the row-major order even in the grid displayed
				/*
				(x, y)
					y=0	y=1
				x=0	0,0	0,1	0,2	0,3
				x=1	1,0	1,1	1,2	1,3
					......
				*/
				glm::vec3 pos = glm::vec3(
								topLeftPosition.y + (y * particleDistance),
								topLeftPosition.x - (x * particleDistance),
								topLeftPosition.x - (x * particleDistance));
				particles[(x*dim) + y] = Particle(pos, m); // Linearization of the index, row = X, col = Y and row dimension = dim
			}
		}

		for(int x=0; x < dim; x++)
		{
			for(int y=0; y < dim; y++)
			{
				// this will create 2 times the constraints since every particles make constraints in all the directions
				// Connecting immediate neighbor particles with constraints (distance 1)
				// if (y +1 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x, y+1, dim), particleDistance);	// RIGHT
				// if (y - 1 >= 0) makeConstraint(getParticle(x, y, dim), getParticle(x, y-1, dim), particleDistance);	// LEFT
				// if (x +1 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x+1, y, dim), particleDistance);	// BOT
				// if (x - 1 >= 0) makeConstraint(getParticle(x, y, dim), getParticle(x-1, y, dim), particleDistance);	// TOP

				// // Constraints on the 4 diagonals
				// if (x +1 < dim && y + 1 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x+1, y+1, dim), particleDistance*glm::sqrt(2.0f));
				// if (x + 1 < dim && y - 1 >= 0) makeConstraint(getParticle(x, y, dim), getParticle(x+1, y-1, dim), particleDistance*glm::sqrt(2.0f));
				// if (x -1 >= 0  && y + 1 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x-1, y+1, dim), particleDistance*glm::sqrt(2.0f));
				// if (x -1 >= 0 	&& y - 1 >= 0) makeConstraint(getParticle(x, y, dim), getParticle(x-1, y-1, dim), particleDistance*glm::sqrt(2.0f));			

				/*
				// Constraints from top left
				// * ---
				// | \
				// |  \
				*/

				if(y+1 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x, y+1, dim), particleDistance);
				if(x+1 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x+1, y, dim), particleDistance);
				if(y+1 < dim && x+1 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x+1, y+1, dim), particleDistance*glm::sqrt(2.0f));

				// if(y+2 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x, y+2, dim), particleDistance);
				// if(x+2 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x+2, y, dim), particleDistance);
				// if(y+2 < dim && x+2 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x+2, y+2, dim), particleDistance*glm::sqrt(2.0f));

			}
		}

		if(pinned){
			// Lock the upper left most three particles and right most three particles
			for(int i=0 ; i<3 ; i++)
			{
				this->particles[0 + i ].makeUnmovable(); 
				this->particles[0 + (dim - 1 -i)].makeUnmovable();
			}
		}

		// std::vector<Constraint>::iterator constraint;
		// int i = 0;
		// for(constraint = constraints.begin(); constraint != constraints.end(); constraint++ )
		// {							
		// 	std::cout << "Constraint :" << i << ": P1: " << FindIndexParticle(*constraint->p1)  << " P2: " << FindIndexParticle(*constraint->p2) << std::endl;
		// 	i++;
		// }
		


		SetUp();
	}
	~Cloth()
	{

		freeGPUresources();
	}

	void PhysicsSteps(float deltaTime, glm::vec3 ballCenterWorld, float ballRadius, float planeLimit)
	{
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->PhysicStep(deltaTime); // calculate the position of each particle at the next time step.
		}
		
		std::vector<Constraint>::iterator constraint;
		for(size_t i=0; i < this->constraintIterations; i++) // iterate over all constraints several times
		{
			for(constraint = constraints.begin(); constraint != constraints.end(); constraint++ )
			{							
				switch(springsType){
					case POSITIONAL:
						constraint->satisfyPositionalConstraint(K); // satisfy constraint.
						break;
					case PHYSICAL:
						constraint->satisfyPhysicsConstraint(K); // satisfy constraint.
						break;
					case POSITIONAL_ADVANCED:
						constraint->satisfyAdvancedPositionalConstraint(K, U, deltaTime);
						break;
					case PHYSICAL_ADVANCED:
						constraint->satisfyAdvancedPhysicalConstraint(K, U, deltaTime);
						break;
				}
			}
		}


		for(size_t i = 0; i < this->collisionIterations; i++){
			for(particle = particles.begin(); particle != particles.end(); particle++)
			{
				particle->SphereCollision(transform->modelMatrix, ballCenterWorld, ballRadius); // calculate the position of each particle at the next time step.
				particle->PlaneCollision(planeLimit);
			}
		}		
	}

	void PhysicsSteps(glm::vec3 ballCenterWorld, float ballRadius, float planeLimit)
	{

		std::vector<Constraint>::iterator constraint;
		for(size_t i=0; i < this->constraintIterations; i++) // iterate over all constraints several times
		{
			for(constraint = constraints.begin(); constraint != constraints.end(); constraint++ )
			{							
				switch(springsType){
					case POSITIONAL:
						constraint->satisfyPositionalConstraint(K); // satisfy constraint.
						break;
					case PHYSICAL:
						constraint->satisfyPhysicsConstraint(K); // satisfy constraint.
						break;
					case POSITIONAL_ADVANCED:
						constraint->satisfyAdvancedPositionalConstraint(K, U, FIXED_TIME_STEP);
						break;
					case PHYSICAL_ADVANCED:
						constraint->satisfyAdvancedPhysicalConstraint(K, U, FIXED_TIME_STEP);
						break;
				}
			}
		}

		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->PhysicStep(); // calculate the position of each particle at the next time step.
		}

		for(size_t i = 0; i < this->collisionIterations; i++){
			for(particle = particles.begin(); particle != particles.end(); particle++)
			{
				particle->SphereCollision(transform->modelMatrix, ballCenterWorld, ballRadius); // calculate the position of each particle at the next time step.
				particle->PlaneCollision(planeLimit);
			}
		}		
	}

	void PhysicsSteps(SphereCollider sphere, float planeLimit)
	{
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->PhysicStep(); // calculate the position of each particle at the next time step.
		}
		
		std::vector<Constraint>::iterator constraint;
		for(size_t i=0; i < this->constraintIterations; i++) // iterate over all constraints several times
		{
			for(constraint = constraints.begin(); constraint != constraints.end(); constraint++ )
			{							
				switch(springsType){
					case POSITIONAL:
						constraint->satisfyPositionalConstraint(K);
						break;
					case PHYSICAL:
						constraint->satisfyPhysicsConstraint(K);
						break;
					case POSITIONAL_ADVANCED:
						constraint->satisfyAdvancedPositionalConstraint(K, U, FIXED_TIME_STEP);
						break;
					case PHYSICAL_ADVANCED:
						constraint->satisfyAdvancedPhysicalConstraint(K, U, FIXED_TIME_STEP);
						break;
				}
			}
		}

		for(size_t i = 0; i < this->collisionIterations; i++){
			for(particle = particles.begin(); particle != particles.end(); particle++)
			{
				// for(sphere = spheres.begin(); sphere != spheres.end(); sphere++){
				// 	//particle->SphereCollision(sphere.transform->modelMatrix, sphere.Position(), sphere.radius); // calculate the position of each particle at the next time step.
				// }
				
				particle->SphereCollision(sphere, transform->modelMatrix);
				particle->PlaneCollision(planeLimit);
			}
		}		
	}

	void AddGravityForce(){
		glm::vec3 gravityVec = glm::vec3(0.0f, 1.0f * (gravityForce), 0.0f);
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->addForce(gravityVec * particle->mass); // add the forces to each particle
		}
	}
	void AddRandomIntensityForce(glm::vec3 normalizedDirection, float min, float max)
	{
		std::vector<Particle>::iterator particle;
		glm::vec3 force = glm::normalize(normalizedDirection);
		srand(time(0));
		float randomIntensity;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			randomIntensity = min + ((max - min) * (rand()%2));
			force *= randomIntensity;
			particle->addForce(force); // add the forces to each particle
		}
	}
	void AddForceToAllParticles(const glm::vec3 forceVector)
	{
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->addForce(forceVector); // add the forces to each particle
		}
	}
	void windForce(glm::vec3 direction)
	{
		std::vector<Particle>::iterator particle;
		bool change = false;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			glm::vec3 forceToApply = direction;
			if(change){
				forceToApply *= 1.5f;
			} else {
				forceToApply *= 0.5f;
			}
			particle->addForce(direction); // add the forces to each particle
			change = !change;
		}
	}

	void Draw()
	{
		//SetUp();
		UpdateNormals();
		UpdateBuffers();

		glBindVertexArray(this->VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void ResetShaderForce(){
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++){
			particle->shader_force = glm::vec3(0.0f);
		}
	}

	void PrintParticles(unsigned int times)
	{
		Particle p;
		std::cout << "Print number: " << times << " ----------------------------" << std::endl;
		std::cout << "Constraints number: " << constraints.size() << std::endl;
		UpdateNormals();
		for(int x=0; x < dim; x++)
		{
			for(int y=0; y < dim; y++)
			{
				std::cout << std::endl;
				std::cout << "Normal: " << getParticle(x, y, dim)->normal.x << ", " << getParticle(x, y, dim)->normal.y << ", " << getParticle(x, y, dim)->normal.z  << std::endl;
				std::cout << "Position: " << getParticle(x, y, dim)->pos.x << ", " << getParticle(x, y, dim)->pos.y << ", " << getParticle(x, y, dim)->pos.z << std::endl;
			}
		}
	}

	void DeleteAllConstraintOfParticle(Particle* pToDelete){
		bool del = false;
		for(size_t i = 0;  i < constraints.size(); i++){
			if(constraints[i].p1->pos.x == pToDelete->pos.x && constraints[i].p1->pos.y == pToDelete->pos.y)
				del = true;
			if(constraints[i].p2->pos.x == pToDelete->pos.x && constraints[i].p2->pos.y == pToDelete->pos.y)
				del = true;

			if(del)
				constraints.erase(constraints.begin() + i);
			
			del = false;
		}

		pToDelete->renderable = false;

		// for(size_t x = 0; x < particles.size(); x++){
		// 	for(size_t y = 0; y < particles.size(); y++){
		// 		if(getParticle(x, y, particles.size()) == pToDelete)
		// 			particles.erase(particles.begin() + (x*particles.size()+y));
		// 	}
		// }


		std::cout << "Del: " << del << std::endl;
	}
	void CutAHole(unsigned int x, unsigned int y){
		Particle* pToDelete0 = getParticle(x, y, dim);
		Particle* pToDelete1 = getParticle(x, y-1, dim);
		Particle* pToDelete2 = getParticle(x, y+1, dim);
		Particle* pToDelete3 = getParticle(x+1, y, dim);
		Particle* pToDelete4 = getParticle(x+1, y+1, dim);
		Particle* pToDelete5 = getParticle(x+1, y-1, dim);
		Particle* pToDelete6 = getParticle(x-1, y, dim);
		Particle* pToDelete7 = getParticle(x-1, y+1, dim);
		Particle* pToDelete8 = getParticle(x-1, y-1, dim);

		DeleteAllConstraintOfParticle(pToDelete0);
		DeleteAllConstraintOfParticle(pToDelete1);
		DeleteAllConstraintOfParticle(pToDelete2);
		DeleteAllConstraintOfParticle(pToDelete3);
		DeleteAllConstraintOfParticle(pToDelete4);
		DeleteAllConstraintOfParticle(pToDelete5);
		DeleteAllConstraintOfParticle(pToDelete6);
		DeleteAllConstraintOfParticle(pToDelete7);
		DeleteAllConstraintOfParticle(pToDelete8);
	}

};
