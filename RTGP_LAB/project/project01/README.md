# **CLOTH SIMULATION**

# Summary

Simulate a cloth, fixed on top-left and top-right corner.
Add a sphere in the scene and make it go through the cloth.
Add Wind force to the cloth.

Prof req. => Add some sort of color modification based on the force or displacement of the single particles of the cloth, in shaders.

(markdown to pdf: https://md2pdf.netlify.app/)

# **Components**

## **Mesh**
The cloth is not rendered using a mesh but simply by coloring each vertex of the same triangle,
Can we render a mesh? (Could be simply a large plane)

## **Cloth**

The cloth is simulated by a matrix of particles, between each particle, distance 1 and distance 2, there will be a spring

### **SPRING**


![Spring](D:\ANDREA\UNI\MAGISTRALE\ANNO_2\II_Semestre\REAL-TIME_GP\RTGP\RTGP_LAB\project\project01\Presentation\SpringMassDamp.png)



$\ 1+2 $


```cpp 
glm::vec3 CalculateCorrectionVector(float K){
        glm::vec3 p1_to_p2 = this->p2->getPos() - this->p1->getPos(); 
        // current distance between p1 and p2
        float current_distance = glm::length(p1_to_p2); 
        p1_to_p2 /= current_distance;

        float delta = current_distance - rest_distance;

        glm::vec3 correctionVector = p1_to_p2 * delta * K; 
        return correctionVector;
}

```


### **SPRING-MASS-DAMP**



```cpp 
glm::vec3 CalculateAdvancedCorrectionVector(float K, float deltaTime){
        
        // (k * ((mag - l)/l) + u (v2 -v1) * n
        // u friction coeff
        float u = 0.05f;

        // vector from p1 to p2
        glm::vec3 p1_to_p2 = this->p2->getPos() - this->p1->getPos(); 
        float current_distance = glm::length(p1_to_p2);
        p1_to_p2 /= current_distance;	// Normalize

        // scalar "a"
        //k[(current_distance -l)/l]
        float a = K * ((current_distance-rest_distance)/rest_distance);

        // vector "B"
        // u * (v_p2 - v_p1) * p1_to_p2
        glm::vec3 v_1 = (this->p1->old_pos - this->p1->pos) / deltaTime; 
        glm::vec3 v_2 = (this->p2->old_pos - this->p2->pos) / deltaTime;
        glm::vec3 b = u * (v_1 - v_2) * p1_to_p2;

        glm::vec3 correctionVector = (a + b) * p1_to_p2;
        return correctionVector;
}
```

## **Physic System - Verlet Integration**

To calculate the forces that will be added the particles we use the Verlet Integration.

### **Forces**

- **Gravity**
- **Wind**

## **Collisions**

COLLISIONS_ITERATIONS:  Number of times that the collision calculation is repeated

### **Detection**
Since the only other object in the scene is a sphere, the collision with the sphere can be trivially found by comparing the distance of the particle with the centre of the sphere and the radius. (if its magnitude is less => COLLISION FOUND)

### **Response**
Project the particle on the surface of the sphere, with a displacement big enough that the problem of triangle-sphere cannot happen


# Implementation


- Generate the cloth particles.
- Add constraints to a particle and its particles at distance 1 and distance 2.
- Load the model of the sphere

## Rendering Loop

- Move sphere
- Detect Collisions
- Respond to the collisions
- Adjust the distance of the particles based on their *distance* and the *standard_distance*


# **Classes**

## **Particle**

This class represent the single particles that will form the grid of the cloth. Each particle is in charge of:
- **Detecting** its **collision** with the sphere and the plane.
- **Reacting** to the **collision**, by offsetting its position.
- Apply **Physic** to the particle by calculating the results of the forces applied to it, using the Verlet integration method. 

- **Position**
- **Normal**


For Verlet integration


- **Old Position**
- **Mass**
- **Acceleration**

For rendering

- **Accumulated Normal**
- **Renderable**


### **Methods**

- **``` void addForce(glm::vec3 f) ```**
        
        This method is used to add a fore vector to

- **```	void PhysicStep(float deltaTime) ```**

        This method is used to applying the forces accumulated to the particles, using Verlet Integration method.

- **``` void BallCollision(glm::mat4 matrixToWorld, const glm::vec3 centerWorld,const float radius) ```**

        Detect the possible collision with the ball
        Respond to the collision



## **Constraints**

CONSTRAINTS_ITERATIONS:  Number of times that the constraints calculation is repeated

- **Particle 1**
- **Particle 2**


Methods

- **Satisfy Constraint()**


## **Cloth**

- vector of **Particles**
- vector of **Constraints**


Methods

### **Step**

- **Physics Steps()**

        Foreach Particles: do the Physic Step
        Foreach Particles: **Satisfy Constraint()**

- **Wind Force()**

        Foreach Particles: add wind force

- **Ball Collisions()**

        Foreach Particles: Ball Collision

- **Draw()**

        Foreach Particles: 
                - Divide the quad of the matrix into 2 triangles


# **FINAL SCENE**

- Use P to respawn the cloth and pin-unpin

- Cube map (
https://sketchfab.com/3d-models/free-cubemap-sphere-45d9cf7df0ad481eab7ca199d458d825
)
- a couple of shaders
- performance (window method)
- cloth
- sphere (texture)
- plane (texture)

## **Scenes**


### **Scene 1**
Single static sphere and a plane


### **Scene 2**
Multiple spheres and a plane


### **Scene 3**
Shooting spheres and cutting the cloth