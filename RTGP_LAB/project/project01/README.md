# **CLOTH SIMULATION**

# Summary

Simulate a cloth, fixed on top-left and top-right corner.
Add a sphere in the scene and make it go through the cloth.
Add Wind force to the cloth.

Prof req. => Add some sort of color modification based on the force or displacement of the single particles of the cloth, in shaders.

## FIX

## AddForce

Add Force is used for both adding a force, like the wind and adding gravity
BUT 
The AddForce used for gravity, iterates on every cloth particle, but the name of the method is the same,
better to change the nam in something like, **addGravityToAllParticles**

# **Components**

## **Mesh**
The cloth is not rendered using a mesh but simply by coloring each vertex of the same triangle,
Can we render a mesh? (Could be simply a large plane)

## **Cloth**

The cloth is simulated by a matrix of particles, between each particle, distance 1 and distance 2, there will be a spring

## **Physic System - Verlet Integration**

To calculate the forces that will be added the particles we use the Verlet Integration.

### **Forces**

- **Gravity**
- **Wind**

## **Collisions**

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
