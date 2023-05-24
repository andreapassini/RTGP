#version 410 core

layout (location = 0) in vec3 position;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 interp_UVW;

void main()
{
    // Use the coordinates of the cube as UVW coordinates of the cube texture
    interp_UVW = position;

    vec4 pos = projectionMatrix * viewMatrix * vec4(position, 1.0f);

    // Trick on the coordinates
    // we want the fragment to be in the far background
    // gl_position in clip coordinates
    // after clipping prospective divide, divide gl_position by w
    // so 
    gl_Position = pos.xyww;
    // to have the z of each fragment = 1, so renderred in the back
    
}