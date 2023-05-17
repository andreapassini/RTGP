/*

06_procedural_base.vert: Vertex shader for the examples on procedural texturing. It is equal to 05_uv2color.vert

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2022/2023
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// UV texture coordinates
layout (location = 1) in vec3 normal;
// the numbers used for the location in the layout qualifier are the positions of the vertex attribute
// as defined in the Mesh class

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

// FOr the illumination model
uniform mat3 normalMatrix;

uniform vec3 pointLightPosition;

out vec3 lightDir;
out vec3 vNormal;
out vec3 vViewPosition;

void main()
{
	// Convert the normal in the correct reference system
	vNormal = normalize(normalMatrix * normal);
	// mv => model view
	// modelMat: local to world
	// viewMat: world to view
	vec4 mvPosition = viewMatrix * modelMatrix * vec4(position, 1.0f);

	// Transform pointLight from world to view
	vec4 vLightPos = viewMatrix * vec4(pointLightPosition, 1.0f);

	lightDir = normalize(vLightPos.xyz - mvPosition.xyz);

	// In view space, Camera is at the origin
	vViewPosition = - mvPosition.xyz;

	// from view to image plane
    gl_Position = projectionMatrix * mvPosition;

}
