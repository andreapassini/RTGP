#pragma once

#include <utils/constraint.h>
#include <vector>
#include <glad/glad.h>

// GLFW
#include <glfw/glfw3.h>

#include <cstdlib>
#include <random>
#include <ctime>

class Cloth
{
private:

	int dim; // number of particles in "width" direction
	// total number of particles is dim*dim

	std::vector<Constraint> constraints; // alle constraints between particles as part of this cloth

	GLuint VAO;
	GLuint EBO;
	GLuint VBO;
    std::vector<GLuint> indices;

	Particle* getParticle(int x, int y) {return &particles[x*dim + y];}
	void makeConstraint(Particle *p1, Particle *p2) {constraints.push_back(Constraint(p1,p2));}


	/* A private method used by drawShaded() and addWindForcesForTriangle() to retrieve the  
	normal vector of the triangle defined by the position of the particles p1, p2, and p3.
	The magnitude of the normal vector is equal to the area of the parallelogram defined by p1, p2 and p3
	*/
	glm::vec3 CalculateNormalSquare(float x, float y)
	{
		if(x + 1 >= dim &&
			y + 1 >= dim)
			return glm::vec3(0.0f);

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

		glm::vec3 pos1 = getParticle( x     , y		)->pos;
		glm::vec3 pos2 = getParticle((x + 1), y		)->pos;
		glm::vec3 pos3 = getParticle( x		, (y + 1))->pos;
		glm::vec3 pos4 = getParticle((x + 1), (y + 1))->pos;

		/*
		   ^
			\
			 \
		*/
		glm::vec3 diagonal1 = pos3-pos1;
		
		/*
			  ^				  
			 /
			/
		*/
		glm::vec3 diagonal2 = pos2-pos4;

		return glm::cross(diagonal1, diagonal2);
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
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_DYNAMIC_DRAW);
		// we copy data in the VBO - we must set the data dimension, and the pointer to the structure containing the data
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->dim * this->dim * sizeof(Particle), &this->particles[0], GL_DYNAMIC_DRAW);
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
		for(int x = 0; x < dim-1; x++)
		{
			for(int y=0; y < dim-1; y++)
			{
				glm::vec3 color(0,0,0);
				if (x%2) // red and white color is interleaved according to which column number
					color = glm::vec3(0.6f,0.2f,0.2f);
				else
					color = glm::vec3(1.0f,1.0f,1.0f);

				indices.push_back(x*dim +y);
				indices.push_back((x+1)*dim +y);
				indices.push_back((x+1)*dim +(y+1));

				indices.push_back(x*dim +y);
				indices.push_back((x+1)*dim +(y+1));
				indices.push_back(x*dim +(y+1));
			}
		}
	}
	void UpdateNormals(){
		// reset normals (which where written to last frame)
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->resetNormal();
		}

		//create smooth per particle normals by adding up all the (hard) triangle normals that each particle is part of
		for(int x = 0; x < dim-1; x++)
		{
			for(int y=0; y < dim-1; y++)
			{
				glm::vec3 normal = CalculateNormalSquare(x, y);
				getParticle(x,y)->addToNormal(normal);
				getParticle(x+1,y)->addToNormal(normal);
				getParticle(x,y+1)->addToNormal(normal);
				getParticle(x+1,y+1)->addToNormal(normal);
			}
		}

		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			glm::normalize(particle->normal);
		}	
	}
	void UpdateBuffers(){

		// POSITION WORKING WITH THIS SECTION
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->dim * this->dim * sizeof(Particle), &this->particles[0], GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, pos));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, normal));
		glBindVertexArray(0);

		// NORMAL WORKING with only this
		glBindVertexArray(this->VAO);
    	glBufferSubData(GL_ARRAY_BUFFER, 0, this->dim * this->dim * sizeof(Particle), &this->particles[0]);
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
public:
	std::vector<Particle> particles; // all particles that are part of this cloth

	Cloth(float dim, float particleDistance, glm::vec3 topLeftPosition){
		this->dim = dim;

		particles.resize(dim*dim); //I am essentially using this vector as an array with room for num_particles_width*dim particles
		
		// creating particles in a grid of particles from (0,0,0) to (width,-height,0)
		for(int x=0; x < dim; x++)
		{
			for(int y=0; y < dim; y++)
			{
				glm::vec3 pos = glm::vec3(
								topLeftPosition.x + (x * particleDistance),
								topLeftPosition.y - (y * particleDistance),
								0);
				particles[x*dim + y] = Particle(pos); // Linearization of the index, row = X, col = Y and row dimension = dim
			}
		}

		// Connecting immediate neighbor particles with constraints (distance 1 and sqrt(2) in the grid)
		for(int x=0; x < dim; x++)
		{
			for(int y=0; y < dim; y++)
			{
				if (x +1 < dim) makeConstraint(getParticle(x,y),getParticle(x+1, y));
				if (x -1 < dim) makeConstraint(getParticle(x,y),getParticle(x-1, y));
				if (y +1 < dim) makeConstraint(getParticle(x,y),getParticle(x, y+1));
				if (y -1 < dim) makeConstraint(getParticle(x,y),getParticle(x, y-1));
			}
		}

		// Constraints on the 4 diagonals
		for(int x=0; x<dim; x+=2)
		{
			for(int y=0; y<dim; y+=2)
			{
				if (x +1 < dim && y + 1 < dim) makeConstraint(getParticle(x,y),getParticle(x+1 ,y+1));
				if (x +1 < dim && y - 1 < dim) makeConstraint(getParticle(x,y),getParticle(x+1, y-1));
				if (x -1 < dim && y + 1 < dim) makeConstraint(getParticle(x,y),getParticle(x-1, y+1));
				if (x -1 < dim && y - 1 < dim) makeConstraint(getParticle(x,y),getParticle(x-1, y-1));			
			}
		}

		// making the upper left most three and right most three particles unmovable
		for(int i=0 ; i<2 ; i++)
		{
			getParticle(0+i ,0)->makeUnmovable(); 
			getParticle(dim-1-i ,0)->makeUnmovable();
		}

		SetUp();
	}
	~Cloth()
	{

		freeGPUresources();
	}

	/* drawing the cloth as a smooth shaded (and colored according to column) OpenGL triangular mesh
	Called from the display() method
	The cloth is seen as consisting of triangles for four particles in the grid as follows:

	(x,y)   *--* (x+1,y)
	        | /|
	        |/ |
	(x,y+1) *--* (x+1,y+1)

	*/
	// void drawShaded()
	// {
	// 	// reset normals (which where written to last frame)
	// 	std::vector<Particle>::iterator particle;
	// 	for(particle = particles.begin(); particle != particles.end(); particle++)
	// 	{
	// 		(*particle).resetNormal();
	// 	}

	// 	//create smooth per particle normals by adding up all the (hard) triangle normals that each particle is part of
	// 	for(int x = 0; x<dim-1; x++)
	// 	{
	// 		for(int y=0; y<dim-1; y++)
	// 		{
	// 			glm::vec3 normal = CalculateNormalSquare(getParticle(x+1,y),getParticle(x,y),getParticle(x,y+1));
	// 			getParticle(x+1,y)->addToNormal(normal);
	// 			getParticle(x,y)->addToNormal(normal);
	// 			getParticle(x,y+1)->addToNormal(normal);

	// 			normal = CalculateNormalSquare(getParticle(x+1,y+1),getParticle(x+1,y),getParticle(x,y+1));
	// 			getParticle(x+1,y+1)->addToNormal(normal);
	// 			getParticle(x+1,y)->addToNormal(normal);
	// 			getParticle(x,y+1)->addToNormal(normal);
	// 		}
	// 	}

	// 	glBegin(GL_TRIANGLES);
	// 	for(int x = 0; x<dim-1; x++)
	// 	{
	// 		for(int y=0; y<dim-1; y++)
	// 		{
	// 			glm::vec3 color(0,0,0);
	// 			if (x%2) // red and white color is interleaved according to which column number
	// 				color = glm::vec3(0.6f,0.2f,0.2f);
	// 			else
	// 				color = glm::vec3(1.0f,1.0f,1.0f);

	// 			drawTriangle(getParticle(x+1,y),getParticle(x,y),getParticle(x,y+1),color);
	// 			drawTriangle(getParticle(x+1,y+1),getParticle(x+1,y),getParticle(x,y+1),color);
	// 		}
	// 	}
	// 	glEnd();
	// }

	/* this is an important methods where the time is progressed one time step for the entire cloth.
	This includes calling satisfyConstraint_Physics() for every constraint, and calling timeStep() for all particles
	*/
	void PhysicsSteps(float deltaTime)
	{
		std::vector<Constraint>::iterator constraint;
		for(int i=0; i < CONSTRAINT_ITERATIONS; i++) // iterate over all constraints several times
		{
			for(constraint = constraints.begin(); constraint != constraints.end(); constraint++ )
			{
				constraint->satisfyConstraint_Physics(); // satisfy constraint.
			}
		}

		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->PhysicStep(deltaTime); // calculate the position of each particle at the next time step.
		}

		UpdateNormals();
		UpdateBuffers();
	}

	void AddGravityForce(){
		glm::vec3 gravityVec = glm::vec3(0.0f, 1.0f * (-0.2f), 0.0f);

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

	/* used to add gravity (or any other arbitrary vector) to all particles*/
	void AddForceToAllParticles(const glm::vec3 forceVector)
	{
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->addForce(forceVector); // add the forces to each particle
		}
	}

	/* used to add wind forces to all particles, is added for each triangle since the final force is proportional to the triangle area as seen from the wind direction*/
	void windForce(const glm::vec3 direction)
	{
		for(int x = 0; x<dim-1; x++)
		{
			for(int y=0; y<dim-1; y++)
			{
			}
		}
	}

	/* used to detect and resolve the collision of the cloth with the ball.
	This is based on a very simples scheme where the position of each particle is simply compared to the sphere and corrected.
	This also means that the sphere can "slip through" if the ball is small enough compared to the distance in the grid bewteen particles
	*/
	void BallCollisions(const glm::vec3 center,const float radius){
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->BallCollision(center, radius);
		}
	}

	void Draw()
	{
		UpdateNormals();
		UpdateBuffers();

		glBindVertexArray(this->VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void PrintParticles(unsigned int times)
	{
		std::vector<Particle>::iterator particle;
		std::cout << "Print number: " << times << " ----------------------------" << std::endl;
		std::cout << "Constraints number: " << constraints.size() << std::endl;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			std::cout << std::endl;
			std::cout << "Force: " << particle->force.x << ", " << particle->force.y << ", " << particle->force.z  << std::endl;
			std::cout << "Position: " <<particle->pos.x << ", " << particle->pos.y << ", " << particle->pos.z << std::endl;
		}
	}
};
