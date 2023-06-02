#version 410 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec3 force;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 colorForce;

void main()
{
		// transformations are applied to each vertex
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
		// transformations are applied to the normal
    colorForce = normalize(force);
}
