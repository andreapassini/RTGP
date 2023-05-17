# Shaders

## Subroutines

OpneGL is a state machine, different subroutines can use the same or not uniforms, but all of  them has to be decaltred and passed on th emain

# Lighting

In the fragment shader, everything need ot be expressed in respect ot the camera, in view coordinate;

### Position

5 - 10 - 10 

right and back

### Plane

For the plane just use Lambert

## Phong

- **l**: Compute the vector from p to camera in **view space**
- **v**: Compute the vector from p to point light **in view space**

## Blinn-Phong

Use the angle between *h* and *n*

- **h**: sum the 2 vector (to get the one in between)

## GGX

  GGX = BRDF * Li * NdotL
  Li = 1
  BRDF = Lambert + specular
  NdotL

# Camera

Yaw: angle around y
Pitch: angle around x
No rotation around Z

use trigonometry to establish front X, Y , Z as a combination of cos and sin of the this->Yaw, this->Pitch

Events for the key, are called only 1 key at the time, use the other ehtod, mapping the entire key as bool