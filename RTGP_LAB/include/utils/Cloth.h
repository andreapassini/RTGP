#pragma once

#include <utils/constraint.h>
#include <vector>
#include <glad/glad.h>

// GLFW
#include <glfw/glfw3.h>
#include <utils/Transform.h>

#include <cstdlib>
#include <random>
#include <ctime>
#include <iostream>

class Cloth
{
private:
	Transform *transform;
	int dim; // number of particles in "width" direction
	// total number of particles is dim*dim

	std::vector<Constraint> constraints; // alle constraints between particles as part of this cloth
	bool springs;
	unsigned int constraintIterations;
	unsigned int collisionIterations;
	float gravityForce;

	GLuint VAO;
	GLuint EBO;
	GLuint VBO;
    std::vector<GLuint> indices;

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
	/* A private method used by drawShaded(), that draws a single triangle p1,p2,p3 with a color*/
	void drawTriangle(Particle *p1, Particle *p2, Particle *p3, const glm::vec3 color)
	{
		glColor3fv( (GLfloat*) &color );

		glNormal3fv((GLfloat *) &(glm::normalize(p1->getNormal())));
		glVertex3fv((GLfloat *) &(p1->getPos() ));

		glNormal3fv((GLfloat *) &(glm::normalize(p2->getNormal())));
		glVertex3fv((GLfloat *) &(p2->getPos() ));

		glNormal3fv((GLfloat *) &(glm::normalize(p3->getNormal())));
		glVertex3fv((GLfloat *) &(p3->getPos() ));
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
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	std::vector<Particle> particles; // all particles that are part of this cloth

	Cloth(int dim, float particleDistance, glm::vec3 topLeftPosition, Transform *t, bool pinned, bool usePhysicConstraints, float k, unsigned int contraintIt, float gravity, unsigned int collisionIt){
		this->dim = dim;
		this->transform = t;
		this->springs = usePhysicConstraints;
		this->constraintIterations = contraintIt;
		this->collisionIterations = collisionIt;
		this->gravityForce = gravity;

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
				particles[(x*dim) + y] = Particle(pos); // Linearization of the index, row = X, col = Y and row dimension = dim
			}
		}

		for(int x=0; x < dim; x++)
		{
			for(int y=0; y < dim; y++)
			{
				// Connecting immediate neighbor particles with constraints (distance 1)
				if (y +1 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x, y+1, dim), particleDistance);	// RIGHT
				if (y - 1 >= 0) makeConstraint(getParticle(x, y, dim), getParticle(x, y-1, dim), particleDistance);	// LEFT
				if (x +1 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x+1, y, dim), particleDistance);	// BOT
				if (x - 1 >= 0) makeConstraint(getParticle(x, y, dim), getParticle(x-1, y, dim), particleDistance);	// TOP

				// Constraints on the 4 diagonals
				if (x +1 < dim && y + 1 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x+1, y+1, dim), particleDistance*glm::sqrt(2.0f));
				if (x + 1 < dim && y - 1 >= 0) makeConstraint(getParticle(x, y, dim), getParticle(x+1, y-1, dim), particleDistance*glm::sqrt(2.0f));
				if (x -1 >= 0  && y + 1 < dim) makeConstraint(getParticle(x, y, dim), getParticle(x-1, y+1, dim), particleDistance*glm::sqrt(2.0f));
				if (x -1 >= 0 	&& y - 1 >= 0) makeConstraint(getParticle(x, y, dim), getParticle(x-1, y-1, dim), particleDistance*glm::sqrt(2.0f));			
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

	void PhysicsSteps(float deltaTime, glm::vec3 ballCenterWorld, float ballRadius, float planeLimit)
	{
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->PhysicStep(deltaTime); // calculate the position of each particle at the next time step.
		}

		if(springs){
			std::vector<Constraint>::iterator constraint;
			for(size_t i=0; i < this->constraintIterations; i++) // iterate over all constraints several times
			{
				for(constraint = constraints.begin(); constraint != constraints.end(); constraint++ )
				{
					constraint->satisfyConstraint_Physics(); // satisfy constraint.
				}
			}
		} else {
			std::vector<Constraint>::iterator constraint;
			for(size_t i=0; i < this->constraintIterations; i++) // iterate over all constraints several times
			{
				for(constraint = constraints.begin(); constraint != constraints.end(); constraint++ )
				{
					constraint->satisfyConstraint(); // satisfy constraint.
				}
			}
		}

		for(size_t i = 0; i < this->collisionIterations; i++){
			for(particle = particles.begin(); particle != particles.end(); particle++)
			{
				particle->BallCollision(transform->modelMatrix, ballCenterWorld, ballRadius); // calculate the position of each particle at the next time step.
				particle->PlaneCollision(planeLimit);
			}
		}

		UpdateNormals();
		UpdateBuffers();
	}
	void AddGravityForce(){
		glm::vec3 gravityVec = glm::vec3(0.0f, 1.0f * (gravityForce), 0.0f);

		AddForceToAllParticles(gravityVec);
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
	void windForce(const glm::vec3 direction)
	{
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->addForce(direction); // add the forces to each particle
		}
	}

	void Draw()
	{
		SetUp();
		UpdateNormals();
		UpdateBuffers();

		glBindVertexArray(this->VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
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
