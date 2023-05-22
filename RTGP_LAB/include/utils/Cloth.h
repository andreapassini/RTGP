#pragma once

#include <utils/constraint.h>
#include <vector>
#include <glad/glad.h>

// GLFW
#include <glfw/glfw3.h>


class Cloth
{
private:

	int num_particles_width; // number of particles in "width" direction
	int num_particles_height; // number of particles in "height" direction
	// total number of particles is num_particles_width*num_particles_height

	std::vector<Constraint> constraints; // alle constraints between particles as part of this cloth

	GLuint VAO;
	GLuint EBO;
	GLuint VBO;
    std::vector<GLuint> indices;

	Particle* getParticle(int x, int y) {return &particles[x*num_particles_width + y];}
	void makeConstraint(Particle *p1, Particle *p2) {constraints.push_back(Constraint(p1,p2));}


	/* A private method used by drawShaded() and addWindForcesForTriangle() to retrieve the  
	normal vector of the triangle defined by the position of the particles p1, p2, and p3.
	The magnitude of the normal vector is equal to the area of the parallelogram defined by p1, p2 and p3
	*/
	glm::vec3 calcTriangleNormal(Particle *p1,Particle *p2,Particle *p3)
	{
		glm::vec3 pos1 = p1->getPos();
		glm::vec3 pos2 = p2->getPos();
		glm::vec3 pos3 = p3->getPos();

		glm::vec3 v1 = pos2-pos1;
		glm::vec3 v2 = pos3-pos1;

		return glm::cross(v1, v2);
	}

	/* A private method used by windForce() to calculate the wind force for a single triangle 
	defined by p1,p2,p3*/
	void addWindForcesForTriangle(Particle *p1,Particle *p2,Particle *p3, const glm::vec3 direction)
	{
		glm::vec3 normal = calcTriangleNormal(p1,p2,p3);
		glm::vec3 d = glm::normalize(normal);
		glm::vec3 force = normal*(glm::dot(d, direction));
		p1->addForce(force);
		p2->addForce(force);
		p3->addForce(force);
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
        glBufferData(GL_ARRAY_BUFFER, num_particles_width * num_particles_height * sizeof(particles[0]), &this->particles[0], GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, pos));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, normal));
	}

	void MakeTriangleFromGrid(){
		for(int x = 0; x<num_particles_width-1; x++)
		{
			for(int y=0; y<num_particles_height-1; y++)
			{
				glm::vec3 color(0,0,0);
				if (x%2) // red and white color is interleaved according to which column number
					color = glm::vec3(0.6f,0.2f,0.2f);
				else
					color = glm::vec3(1.0f,1.0f,1.0f);

				indices.push_back(x*num_particles_height +y);
				indices.push_back((x+1)*num_particles_height +y);
				indices.push_back((x+1)*num_particles_height +(y+1));

				indices.push_back(x*num_particles_height +y);
				indices.push_back((x+1)*num_particles_height +(y+1));
				indices.push_back(x*num_particles_height +(y+1));
			}
		}
	}
public:
	std::vector<Particle> particles; // all particles that are part of this cloth

	Cloth(float dim, float particleDistance, glm::vec3 topLeftPosition){
		num_particles_width = dim;
		num_particles_height = dim;

		particles.resize(dim*dim); //I am essentially using this vector as an array with room for num_particles_width*num_particles_height particles
		
		// creating particles in a grid of particles from (0,0,0) to (width,-height,0)
		for(int x=0; x<num_particles_width; x++)
		{
			for(int y=0; y<num_particles_height; y++)
			{
				glm::vec3 pos = glm::vec3(
								topLeftPosition.x - (x * particleDistance),
								topLeftPosition.y - (y * particleDistance),
								0);
				particles[x*dim + y]= Particle(pos); // insert particle in column x at y'th row
			}
		}

		// Connecting immediate neighbor particles with constraints (distance 1 and sqrt(2) in the grid)
		for(int x=0; x<dim; x++)
		{
			for(int y=0; y<dim; y++)
			{
				if (x<dim-1) makeConstraint(getParticle(x,y),getParticle(x+1,y));
				if (y<dim-1) makeConstraint(getParticle(x,y),getParticle(x,y+1));
				if (x<dim-1 && y<dim-1) makeConstraint(getParticle(x,y),getParticle(x+1,y+1));
				if (x<dim-1 && y<dim-1) makeConstraint(getParticle(x+1,y),getParticle(x,y+1));
			}
		}


		// Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
		for(int x=0; x<dim; x++)
		{
			for(int y=0; y<dim; y++)
			{
				if (x<dim-2) makeConstraint(getParticle(x,y),getParticle(x+2,y));
				if (y<dim-2) makeConstraint(getParticle(x,y),getParticle(x,y+2));
				if (x<dim-2 && y<dim-2) makeConstraint(getParticle(x,y),getParticle(x+2,y+2));
				if (x<dim-2 && y<dim-2) makeConstraint(getParticle(x+2,y),getParticle(x,y+2));			
			}
		}


		// making the upper left most three and right most three particles unmovable
		for(int i=0;i<3; i++)
		{
			getParticle(0+i ,0)->offsetPos(glm::vec3(0.5f,0.0f,0.0f)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
			getParticle(0+i ,0)->makeUnmovable(); 

			getParticle(0+i ,0)->offsetPos(glm::vec3(-0.5f,0.0f,0.0f)); // moving the particle a bit towards the center, to make it hang more natural - because I like it ;)
			getParticle(dim-1-i ,0)->makeUnmovable();
		}

		SetUp();
	}

	/* drawing the cloth as a smooth shaded (and colored according to column) OpenGL triangular mesh
	Called from the display() method
	The cloth is seen as consisting of triangles for four particles in the grid as follows:

	(x,y)   *--* (x+1,y)
	        | /|
	        |/ |
	(x,y+1) *--* (x+1,y+1)

	*/
	void drawShaded()
	{
		// reset normals (which where written to last frame)
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			(*particle).resetNormal();
		}

		//create smooth per particle normals by adding up all the (hard) triangle normals that each particle is part of
		for(int x = 0; x<num_particles_width-1; x++)
		{
			for(int y=0; y<num_particles_height-1; y++)
			{
				glm::vec3 normal = calcTriangleNormal(getParticle(x+1,y),getParticle(x,y),getParticle(x,y+1));
				getParticle(x+1,y)->addToNormal(normal);
				getParticle(x,y)->addToNormal(normal);
				getParticle(x,y+1)->addToNormal(normal);

				normal = calcTriangleNormal(getParticle(x+1,y+1),getParticle(x+1,y),getParticle(x,y+1));
				getParticle(x+1,y+1)->addToNormal(normal);
				getParticle(x+1,y)->addToNormal(normal);
				getParticle(x,y+1)->addToNormal(normal);
			}
		}

		glBegin(GL_TRIANGLES);
		for(int x = 0; x<num_particles_width-1; x++)
		{
			for(int y=0; y<num_particles_height-1; y++)
			{
				glm::vec3 color(0,0,0);
				if (x%2) // red and white color is interleaved according to which column number
					color = glm::vec3(0.6f,0.2f,0.2f);
				else
					color = glm::vec3(1.0f,1.0f,1.0f);

				drawTriangle(getParticle(x+1,y),getParticle(x,y),getParticle(x,y+1),color);
				drawTriangle(getParticle(x+1,y+1),getParticle(x+1,y),getParticle(x,y+1),color);
			}
		}
		glEnd();
	}

	/* this is an important methods where the time is progressed one time step for the entire cloth.
	This includes calling satisfyConstraint() for every constraint, and calling timeStep() for all particles
	*/
	void PhysicsSteps()
	{
		std::vector<Constraint>::iterator constraint;
		for(int i=0; i<CONSTRAINT_ITERATIONS; i++) // iterate over all constraints several times
		{
			for(constraint = constraints.begin(); constraint != constraints.end(); constraint++ )
			{
				constraint->satisfyConstraint(); // satisfy constraint.
			}
		}

		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->PhysicStep(); // calculate the position of each particle at the next time step.
		}
	}

	void AddGravityForce(){
		glm::vec3 gravityVec = glm::vec3(0.0f, -1.0f, 0.0f);
		gravityVec *= 9.8f;
		glm::normalize(gravityVec);

		addForce(gravityVec);
	}

	/* used to add gravity (or any other arbitrary vector) to all particles*/
	void addForce(const glm::vec3 direction)
	{
		std::vector<Particle>::iterator particle;
		for(particle = particles.begin(); particle != particles.end(); particle++)
		{
			particle->addForce(direction); // add the forces to each particle
		}
	}

	/* used to add wind forces to all particles, is added for each triangle since the final force is proportional to the triangle area as seen from the wind direction*/
	void windForce(const glm::vec3 direction)
	{
		for(int x = 0; x<num_particles_width-1; x++)
		{
			for(int y=0; y<num_particles_height-1; y++)
			{
				addWindForcesForTriangle(getParticle(x+1,y),getParticle(x,y),getParticle(x,y+1),direction);
				addWindForcesForTriangle(getParticle(x+1,y+1),getParticle(x+1,y),getParticle(x,y+1),direction);
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
		glBindVertexArray(this->VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
};
