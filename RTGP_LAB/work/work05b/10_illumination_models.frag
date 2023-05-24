/*

10_illumination_models.frag: Fragment shader for the Lambert, Phong, Blinn-Phong and GGX illumination models

N.B. 1)  "09_illumination_models.vert" must be used as vertex shader

N.B. 2)  the different illumination models are implemented using Shaders Subroutines

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2021/2022
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

const float PI = 3.14159265359;

// output shader variable
out vec4 colorFrag;

// light incidence direction (calculated in vertex shader, interpolated by rasterization)
in vec4 worldPosition;
// the transformed normal has been calculated per-vertex in the vertex shader
in vec4 worldNormal;

uniform samplerCube tCube;

// ambient, diffusive and specular components (passed from the application)
uniform vec3 cameraPosition;
uniform vec3 pointLightPosition;
// weight of the components
// in this case, we can pass separate values from the main application even if Ka+Kd+Ks>1. In more "realistic" situations, I have to set this sum = 1, or at least Kd+Ks = 1, by passing Kd as uniform, and then setting Ks = 1.0-Kd
uniform float Eta;
uniform float mFresnelPower;

////////////////////////////////////////////////////////////////////

// the "type" of the Subroutine
subroutine vec4 ill_model();

// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform ill_model Illumination_Model;

////////////////////////////////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the Lambert model
subroutine(ill_model)
vec4 Reflection() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // Compute the reflaction vector and comupute the coordinates of the cubetexture
    vec3 V = normalize(worldPosition.xyz - cameraPosition); // V incoming vector from camera
    vec3 R = normalize(reflect(V, normalize(worldNormal)));

    // Lambert illumination model
    return texture(tCube, R);
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the Lambert model
subroutine(ill_model)
vec4 Fresnel() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    vec3 N = normalize(worldNormal);

    // Compute the reflaction vector and comupute the coordinates of the cubetexture
    vec3 V = normalize(worldPosition.xyz - cameraPosition); // V incoming vector from camera
    vec3 R = normalize(reflect(V, N);)

    // Light incoming direction
    vec3 L = normalize(worldPosition.xyz - pointLightPosition);

    vec3 H = normalize(L+V); // The half vector

    vec4 reflectedColor = texture(tCube, R);

    vec3 refractDir[3];

    vec3 refractDir[0] = refract(V, N, Eta);
    vec3 refractDir[1] = refract(V, N, Eta * 0.99);
    vec3 refractDir[2] = refract(V, N, Eta * 0.98);

    vec4 refractedColor  = vec4(1.0);
    
    refractedColor.r = texture(tCube, refractDir[0]).r;
    refractedColor.g = texture(tCube, refractDir[1]).g;
    refractedColor.b = texture(tCube, refractDir[2]).b;


    float F0 = ((1.0-Eta) * (1.0-Eta))/((1.0+Eta) * (1.0+Eta));

    float Ration = F0 + (1.0-F0)*pow(1.0-max(dot(V,H), 0, 0), mFresnelPower);   // approximation

    // Lambert illumination model
    return mix(refractedColor, reflectedColor, clamp(Ration, 0.0, 1.0));
}
//////////////////////////////////////////

// main
void main(void)
{
    colorFrag = Illumination_Model();
}
