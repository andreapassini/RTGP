#pragma once

#include <utils/constraint.h>
#include <vector>
#include <glad/glad.h>
#include <physicsSimulation/physicsSimulation.h>
#include <utils/particlesToCut.h>

// GLFW
#include <glfw/glfw3.h>
#include <utils/Transform.h>
#include <utils/Scene.h>

#include <cstdlib>
#include <random>
#include <ctime>
#include <iostream>

#define FIXED_TIME_STEP (1.0f / 60.0f)
#define FIXED_TIME_STEP2 (FIXED_TIME_STEP * FIXED_TIME_STEP)


class Cloth
{
private:

	ConstraintType springsType;
	unsigned int constraintIterations;
	unsigned int collisionIterations;
	float gravityForce;
	unsigned int constraintLevel;
	float cuttingDistanceMultiplier;

	GLuint VAO;
	GLuint EBO;
	GLuint VBO;
    std::vector<GLuint> indices;

	float maxForce;
	bool hole;

	void makeConstraint(Particle *p1, Particle *p2, float rest_distance, float cuttingMuliplier) {
		constraints.push_back(Constraint(p1,p2, rest_distance, cuttingDistanceMultiplier));
	}

	glm::vec3 CalculateNormalTriangle(Particle* p1, Particle* p2, Particle* p3){
		glm::vec3 pos1 = p1->pos;
		glm::vec3 pos2 = p2->pos;
		glm::vec3 pos3 = p3->pos;

		glm::vec3 v1 = pos2-pos1;
		glm::vec3 v2 = pos3-pos1;

		glm::vec3 norm = glm::cross(v1, v2);
		glm::normalize(norm);
		return norm;
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

		glm::vec3 norm = glm::cross(diagonal2, diagonal1);
		glm::normalize(norm);


		return norm;
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

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, color));

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
				glm::vec3 normal;
				// normal = CalculateNormalSquare(x, y);
				// getParticle(x,   y,   dim)->addToNormal(normal);
				// getParticle(x+1, y,   dim)->addToNormal(normal);
				// getParticle(x,   y+1, dim)->addToNormal(normal);
				// getParticle(x+1, y+1, dim)->addToNormal(normal);

				normal = CalculateNormalTriangle(getParticle(x, y,   dim), getParticle(x+1,   y,   dim), getParticle(x,   y+1, dim));
				getParticle(x,   y,   dim)->addToNormal(normal);
				getParticle(x+1, y,   dim)->addToNormal(normal);
				getParticle(x,   y+1, dim)->addToNormal(normal);

				// normal = CalculateNormalTriangle(getParticle(x+1, y+1,   dim), getParticle(x+1, y,   dim), getParticle(x,   y+1, dim));
				// getParticle(x+1, y+1, dim)->addToNormal(normal);
				// getParticle(x+1, y  , dim)->addToNormal(normal);
				// getParticle(x,   y+1, dim)->addToNormal(normal);

			}
		}
	}
	void UpdateBuffers(){
		glBindVertexArray(this->VAO);
		// glBufferSubData(GL_ARRAY_BUFFER, 0, this->dim * this->dim * sizeof(particles[0]), &particles);

		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->dim * this->dim * sizeof(Particle), this->particles.data(), GL_DYNAMIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, pos));
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, normal));

		glEnableVertexAttribArray(2);
		if(springsType == PHYSICAL || springsType == PHYSICAL_ADVANCED){
			// glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, force));
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, shader_force));
		} else {
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, shader_force));
		}

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, color));

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

	glm::vec3 FindIndexParticle(Particle* p){
		int linearizedIndex = -1;

		for(size_t i = 0; i <= particles.capacity() && linearizedIndex == -1; i++ )
		{							
			if(&particles[i] == p){
				linearizedIndex = i;
			}
		}

		glm::vec3 index = glm::vec3(-1.0f);

		int x = linearizedIndex / this->dim;
		int y = linearizedIndex % this->dim;

		index = glm::vec3(x, y, -1.0f);
		return index;
	}
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	int dim; // number of particles in "width" direction
	// total number of particles is dim*dim

	Transform *transform;
	
	std::vector<Constraint> constraints; // alle constraints between particles as part of this cloth
	std::vector<Particle> particles; // all particles that are part of this cloth
	float K;
	float U;

	Cloth(int dim, float particleDistance, glm::vec3 topLeftPosition, Transform *t, bool pinned, ConstraintType usePhysicConstraints, float k, float u, unsigned int contraintIt, float gravity, float m, unsigned int collisionIt, unsigned int constraintLevel, float cuttingMultiplier){
		this->dim = dim;
		this->transform = t;
		this->springsType = usePhysicConstraints;
		this->constraintIterations = contraintIt;
		this->collisionIterations = collisionIt;
		this->gravityForce = gravity;
		this->K = k;
		this->U = u;
		this->constraintLevel = constraintLevel;
		this->cuttingDistanceMultiplier = cuttingMultiplier;

		maxForce = 0.0f;
		particles.resize(dim*dim); //I am essentially using this vector as an array with room for num_particles_width*dim particles
		
		const glm::vec3 evenColor (0.467f, 0.259f, 1.0f); 
		const glm::vec3 oddColor (0.467f, 0.259f, 1.0f); 
		glm::vec3 color (0.0f);

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
				if(x % 4 == 0){
					color = evenColor;
				} else {
					color = oddColor;
				}
				particles[(x*dim) + y] = Particle(pos, m, color); // Linearization of the index, row = X, col = Y and row dimension = dim
			}
		}

		for(int x=0; x < dim; x++)
		{
			for(int y=0; y < dim; y++)
			{
				/*
				// Constraints from top left
				// * ---
				// | \
				// |  \
				*/
				for(int i = 1; i <= this->constraintLevel; i++){
					if(y+i < dim) makeConstraint(getParticle(x, y, dim), getParticle(x, y+i, dim), particleDistance * i, cuttingDistanceMultiplier);
					if(x+i < dim) makeConstraint(getParticle(x, y, dim), getParticle(x+i, y, dim), particleDistance * i, cuttingDistanceMultiplier);
					if(y+i < dim && x+i < dim) makeConstraint(getParticle(x, y, dim), getParticle(x+i, y+i, dim), particleDistance*glm::sqrt(2.0f) * i, cuttingDistanceMultiplier);
				}
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
		
		SetUp();
	}
	~Cloth()
	{
		freeGPUresources();
	}

	Particle* getParticle(int x, int y, int rowDim) {return &particles[x*rowDim + y];}

	void PhysicsSteps(Scene* scene)
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
						constraint->satisfyPositionalConstraint(K); // satisfy constraint.
						break;
					case PHYSICAL:
						constraint->satisfyPhysicsConstraint(K); // satisfy constraint.
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
				for(const auto plane : scene->planes){
					particle->PlaneCollision(plane);
				}
				for(const auto sphere : scene->spheres){
					particle->SphereCollision(sphere);
				}
				for(const auto capsule : scene->capsules){
					particle->CapsuleCollision(capsule);
				}
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
		if(hole){
			SetUp();
			hole = false;
		} else {
		}

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
		hole = true;

		// for(size_t x = 0; x < particles.size(); x++){
		// 	for(size_t y = 0; y < particles.size(); y++){
		// 		if(getParticle(x, y, particles.size()) == pToDelete)
		// 			particles.erase(particles.begin() + (x*particles.size()+y));
		// 	}
		// }

		// std::cout << "Del: " << del << std::endl;
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

	void CutAHole(Particle* p){
		unsigned int x, y;

		glm::vec3 index = FindIndexParticle(p);
		x = index.x;
		y = index.y;
		
		CutAHole( x,  y);
	}

	void CheckForCuts(){
		int size = ParticlesToCut::GetInstance()->particles.size();

		if(size > 0){
			hole = true;
			std::cout << "Capacity: " << size << std::endl;

			for(int i = 0; i < size; i++){
				Particle* pToCut = ParticlesToCut::GetInstance()->particles[i];
				CutAHole(pToCut);
				std::cout << "Cutting " << std::endl;
			}

			ParticlesToCut::GetInstance()->particles.clear();
		}


	}
};