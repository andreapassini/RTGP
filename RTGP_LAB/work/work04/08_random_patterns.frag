/*

08_random_patterns.frag: a fragment shader with different noise-based effects

N.B. 1)  "06_procedural_base.vert" must be used as vertex shader

N.B. 2)  the different effects are implemented using Shaders Subroutines

N.B. 3) we use simplex noise implementation from
        https://github.com/stegu/webgl-noise//wiki
        to generate the fragments colors


author: Davide Gadia

Real-Time Graphics Programming - a.a. 2022/2023
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

const float PI = 3.14159265359;

// output shader variable
out vec4 colorFrag;

in vec3 lightDir;
in vec3 vNormal;
in vec3 vViewPosition;

uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform vec3 diffuseColor;

uniform float Ka;
uniform float Ks;
uniform float Kd;

uniform float shininess;
uniform float alpha;
uniform float F0;

// the "type" of the Subroutine
subroutine vec4 ill_model();

// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform ill_model Illumination_Model;

//////////////////////////////////////////
// a subroutine for a simple noise shader
subroutine(ill_model)
vec3 Lambert() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
  vec3 N = normalize(vNormal);
  vec3 L = normalize(lightDir);

  float lambertian = max(dot(L, N), 0); // avoid angles that lead to neg value

  return vec3(Kd * lambertian * diffuseColor);
}
//////////////////////////////////////////

//////////////////////////////////////////
subroutine(ill_model)
vec3 Phong() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
  // ambient + diffuse + specular

  vec3 color = Ka * ambientColor;

  vec3 N = normalize(vNormal);
  vec3 L = normalize(lightDir);

  float lambertian = max(dot(L, N), 0.0); // avoid angles that lead to neg value

  if(lambertian > 0) // Compute the specular only if lambertain in not 0
  { 
    vec3 V = normalize(vViewPosition);
    vec3 R =  reflect(-L, N); // the reflected ray need to be incoming

    float sepcAngle = max(dot(R, V), 0.0); // avoid angles that lead to neg value
    
    float specular = pow(sepcAngle, shininess);

    color += vec3(Kd * lambertian * diffuseColor + Ks * specularColor);
  }

  return color;
}
//////////////////////////////////////////

//////////////////////////////////////////
subroutine(ill_model)
vec3 BlinnPhong() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
  // ambient + diffuse + specular

  vec3 color = Ka * ambientColor;

  vec3 N = normalize(vNormal);
  vec3 L = normalize(lightDir);

  float lambertian = max(dot(L, N), 0.0); // avoid angles that lead to neg value

  if(lambertian > 0) // Compute the specular only if lambertain in not 0
  { 
    vec3 V = normalize(vViewPosition);
    vec3 H =  normalize(L + V); // the reflected ray need to be incoming

    float sepcAngle = max(dot(H, N), 0.0); // avoid angles that lead to neg value
    
    float specular = pow(sepcAngle, shininess);

    color += vec3(Kd * lambertian * diffuseColor + Ks * specularColor);
  }

  return color;
}
//////////////////////////////////////////

//////////////////////////////////////////
float G1(float angle, float alpha)
{
  float r = (alpha + 1.0);
  float k = (r*r) / 8.0;

  float num = angle;
  float denom = angle * (1.0 - k) + k;

  return num / denom;
}

subroutine(ill_model)
vec3 GGX() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
  vec3 N = normalize(vNormal);
  vec3 L = normalize(lightDir);

  float NdotL = max(dot(L, N), 0.0); // avoid angles that lead to neg value
  // Diffusive part
  vec3 lambert = (Kd * diffuseColor) / PI;

  vec3 specular = vec3(0.0);

  if(NdotL > 0) // Compute the specular only if lambertain in not 0
  { 
    vec3 V = normalize(vViewPosition);
    vec3 H =  normalize(L + V); // the reflected ray need to be incoming

    float NdotH = max(dot(H, N), 0.0); // avoid angles that lead to neg value
    float NdotV = max(dot(H, V), 0.0); // avoid angles that lead to neg value
    float VdotH = max(dot(V, H), 0.0); // avoid angles that lead to neg value

    float alpha_squared = alpha*alpha;
    float NdotH_squared = NdotH * NdotH;

    vec3 F = vec3(pow(1.0 - VdotH, 5.0));
    F *= (1.0 - F0);
    F +=  F0;

    float D = alpha_squared;
    float denom = (NdotH_squared * (alpha_squared - 1.0) + 1.0);
    D /= PI * denom * denom;

    float G2 = G1(NdotV, alpha) * G1(NdotL,  alpha);

    specular = (F * D * G2) / (4.0 * NdotV * NdotL);
  }
  // GGX = BRDF * Li * NdotL
  // Li = 1
  // BRDF = Lambert + specular
  return (lambert + specular) * NdotL;
}
//////////////////////////////////////////

// main
void main(void)
{
    // we call the pointer function Random_Patterns():
    // the subroutine selected in the main application will be called and executed

    vec3 color = vec3(Illumination_Model());

  	colorFrag = vec4(color, 1.0f);
}
