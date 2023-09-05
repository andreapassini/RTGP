#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 force;
layout (location = 3) in vec3 color;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

uniform vec3 lightVector;
out vec3 lightDir;

out vec3 vNormal;

out vec3 vViewPosition;

out vec2 interp_UV;
out vec4 posLightSpace;

out vec3 colorForce;

void main()
{
  // vertex position in world coordinates
  vec4 mPosition = modelMatrix * vec4( position, 1.0 );
  // vertex position in camera coordinates
  vec4 mvPosition = viewMatrix * mPosition;

  // view direction, negated to have vector from the vertex to the camera
  vViewPosition = -mvPosition.xyz;

  // transformations are applied to the normal
  vNormal = normalize( normalMatrix * normal );

  // light incidence directions in view coordinate
  lightDir = vec3(viewMatrix  * vec4(lightVector, 0.0));

  // we apply the projection transformation
  gl_Position = projectionMatrix * mvPosition;
  
  colorForce = color;
  posLightSpace = mPosition;
}
