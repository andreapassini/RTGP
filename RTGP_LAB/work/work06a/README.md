# Shadow-Map

Use a void texture to save the depth info at the first step of gpu (Differed rendering )

Using a directional light.

## First rendering step

virtual camera at the light and save in the auz frame buffer the depth map seen by the light

- Model view
- Projection Matrix

of the temporary camera placed in the light position.

Using a direction light it will be an orthographic projection
(Cube) and setting its parameters for:

- near
- far plane
- frustum size 

We fake the position of the light, since it's directional, pointing at the origin (0,0,0)

### Fragment Shader

we don't need to save color, juts the depth in the depth buffer, since we set up it not to render anything

## Second rendering Step

Returning to the STANDARD rendering and us e the frame buffer in the illumination model

REset the frame buffer to use, to the standard one


Pass to the shader the lightSpaceMatrix.

use the depth data, as texture, and bind it as buffer 2


Check if a fragment is present in the depth map
but convert from view space to light space 


Pos light spae is not been perspective divided and set in NDC and normalized

'''
vec3 projCoords = posLightSpace.xyz / posLightSpace.w;

'''

Do that in Frag Shader and we are between (-1, 1) but UV are in range (0,1)


### ERRORS:

#### Shadow Acne
black lines: res of the shadow texture and res of the original texture are different, numerical errors cause it. 

Solved by adding a value, a bias, a small amount to the current depth. (0.005 - 0.05)


#### Border color

Outside the frustum we have black color, cause by the default border color that is black. It is assigning black outside the texture

SOl: change broder color

#### Jagged shadow border => Percentage close filtering

#### Black Background

Outside the far plane