#version 410 core

// output shader variable
out vec4 colorFrag;

// the transformed normal has been calculated per-vertex in the vertex shader
in vec3 colorForce;


void main(){
	// we use the normal as a color
	colorFrag = vec4(normalize(colorForce),1.0);
}
