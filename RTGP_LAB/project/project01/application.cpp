/*
OpenGL coordinate system (right-handed)
positive X axis points right
positive Y axis points up
positive Z axis points "outside" the screen


                              Y
                              |
                              |
                              |________X
                             /
                            /
                           /
                          Z
*/

#include <string>
#include <iostream>
#include <chrono>

// Loader for OpenGL extensions
// http://glad.dav1d.de/
// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>

#include <glfw/glfw3.h>

#ifdef _WINDOWS_
    #error windows.h was included!
#endif

#include <utils/shader.h>
#include <utils/model.h>
#include <utils/camera.h>
#include <utils/performanceCalculator.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <utils/Transform.h>
#include <utils/cloth.h>

// we include the library for images loading
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image/stb_image.h"

#define NR_LIGHTS 3


GLFWwindow* window;
GLuint screenWidth = 1200, screenHeight = 900;

int SetupOpenGL();

void SetupShaders();
void DeleteShaders();
void PrintCurrentShader(int shader);

void SetupBlinPhong();

GLint LoadTexture(const char* path);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void apply_camera_movements();
bool keys[1024];
bool R_KEY = false;
GLfloat lastX, lastY;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLfloat orientationY = 0.0f;
GLfloat spin_speed = 30.0f;
GLboolean spinning = GL_TRUE;
GLfloat positionZ = 0.0f;
GLfloat movement_speed = 5.0f;
GLboolean movingOnX = GL_TRUE;
GLboolean instantiate = GL_FALSE;

GLboolean wireframe = GL_FALSE;

Camera camera(glm::vec3(0.0f, -2.0f, 7.0f), GL_FALSE);

enum available_ShaderPrograms{ FULLCOLOR, FLATTEN, NORMAL2COLOR, WAVE, BLINPHONG };
const char * print_available_ShaderPrograms[] = { "FULLCOLOR", "FLATTEN", "NORMAL2COLOR", "WAVE", "BLINPHONG" };
GLuint current_program = FULLCOLOR;
vector<Shader> shaders;

// Uniforms to pass to shaders
// pointlights positions
glm::vec3 lightPositions[] = {
    glm::vec3(5.0f, 10.0f, 10.0f),
    glm::vec3(-5.0f, 10.0f, 10.0f),
    glm::vec3(5.0f, 10.0f, -10.0f),
};

// diffusive, specular and ambient components
GLfloat diffuseColor[] = {1.0f,0.0f,0.0f};
GLfloat specularColor[] = {1.0f,1.0f,1.0f};
GLfloat ambientColor[] = {0.1f,0.1f,0.1f};
// weights for the diffusive, specular and ambient components
GLfloat Kd = 0.5f;
GLfloat Ks = 0.4f;
GLfloat Ka = 0.1f;
// shininess coefficient for Blinn-Phong shader
GLfloat shininess = 25.0f;

// roughness index for GGX shader
GLfloat alpha = 0.2f;
// Fresnel reflectance at 0 degree (Schlik's approximation)
GLfloat F0 = 0.9f;

vector<GLint> textureID;
GLfloat repeat = 1.0f;  // UV repetitions


GLfloat myColor[] = {1.0f,0.0f,0.0f};
GLfloat clothColor[] = {0.0f, 31.0f, 0.0f};
GLfloat coral[] = {1.0f, 0.5f, 0.31f};
GLfloat planeColor[] = {0.13f, 0.07f, 0.34f};

GLfloat weight = 0.2f;
GLfloat speed = 5.0f;

glm::vec3 startingPosition(0.0f, 0.0f, 0.0f);

bool once = true;
unsigned int iter = 0;

bool clothExist = true;
unsigned int prints = 0;
bool pinned = true;
bool usePhysicConstraints = false;
float gravity = -9.8f;
float k = 0.5f;
unsigned int constraintIterations = 15;
unsigned int collisionIterations = 5;

unsigned int windowSize = 10;
unsigned int overlap = 3;

int main()
{
    if(SetupOpenGL() == -1)
        return -1;

    SetupShaders();
    PrintCurrentShader(current_program);

    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    // View matrix (=camera): position, view direction, camera "up" vector
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    Model sphereModel("../../models/sphere.obj");
    Model planeModel("../../models/plane.obj");
    Model cubeModel("../../models/cube.obj");

    textureID.push_back(LoadTexture("../../textures/UV_Grid_Sm.png"));
    textureID.push_back(LoadTexture("../../textures/SoilCracked.png"));

    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    glm::vec3 spherePosition(3.0f, -4.5f, -2.5f);
    Transform sphereTransform(view);
    positionZ = 0.8f;
    GLint directionZ = 1;

    Transform planeTransform(view);
    Transform cubeTransform(view);

    Transform clothTransform(view);
    Cloth cloth(30, 0.15f, startingPosition, &clothTransform, pinned, usePhysicConstraints, k, constraintIterations, gravity, collisionIterations);

    PerformanceCalculator performanceCalculator(windowSize, overlap);
    // DELTA TIME using std::chrono
    // https://stackoverflow.com/questions/14391327/how-to-get-duration-as-int-millis-and-float-seconds-from-chrono
    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::duration<float> fsec;
    auto start_time = Time::now();   

    // Rendering loop: this code is executed at each frame
    while(!glfwWindowShouldClose(window))
    {
        // we determine the time passed from the beginning
        // and we calculate time difference between current frame rendering and the previous one
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        auto current_time = Time::now();
        fsec deltaTime = (current_time - start_time);
        start_time = Time::now();

        performanceCalculator.Step(deltaTime.count());

        glfwPollEvents();
        apply_camera_movements();
        // View matrix (=camera): position, view direction, camera "up" vector
        view = camera.GetViewMatrix();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        shaders[current_program].Use();
        // uniforms are passed to the corresponding shader
        if (current_program == FULLCOLOR || current_program == FLATTEN)
        {
            GLint fragColorLocation = glGetUniformLocation(shaders[current_program].Program, "colorIn");
            glUniform3fv(fragColorLocation, 1, myColor);
        }
        else if (current_program == WAVE)
        {
            GLint weightLocation = glGetUniformLocation(shaders[current_program].Program, "weight");
            GLint timerLocation = glGetUniformLocation(shaders[current_program].Program, "timer");
            glUniform1f(weightLocation, weight);
            glUniform1f(timerLocation, currentFrame*speed);
        } else if (current_program == BLINPHONG) {
            SetupBlinPhong();
            
            for (GLuint i = 0; i < NR_LIGHTS; i++)
            {
                string number = to_string(i);
                glUniform3fv(glGetUniformLocation(shaders[current_program].Program, ("lights[" + number + "]").c_str()), 1, glm::value_ptr(lightPositions[i]));
            }

        }

        GLint textureLocation = glGetUniformLocation(shaders[current_program].Program, "tex");
        GLint repeatLocation = glGetUniformLocation(shaders[current_program].Program, "repeat");


        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        // CLOTH
        if (current_program == FULLCOLOR || current_program == FLATTEN)
        {
            GLint fragColorLocation = glGetUniformLocation(shaders[current_program].Program, "colorIn");
            glUniform3fv(fragColorLocation, 1, clothColor);
        }
        
        cloth.AddGravityForce();
        cloth.windForce(glm::vec3(0.0f, 0.0f, 1.0f)*3.5f);
        clothTransform.Transformation(
            glm::vec3(1.0f, 1.0f, 1.0f),
            0.0f, glm::vec3(0.0f, 1.0f, 0.0f),
            startingPosition,
            view
        );
        cloth.PhysicsSteps(deltaTime.count(), (glm::vec4(spherePosition, 1.0f) * sphereTransform.modelMatrix), 1.0f, -9.9f);
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(clothTransform.modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(clothTransform.normalMatrix));
        cloth.Draw();
        
        if(!spinning && once)
        {
            // cloth.~Cloth();
            // pinned = !pinned;
            // new(&cloth) Cloth(30, 0.15f, startingPosition, &clothTransform, pinned, usePhysicConstraints, k, constraintIterations, gravity, collisionIterations);
            once = false;
            // std::cout << "Framerate: " << (int)performanceCalculator.framerate << std::endl;

            cloth.CutAHole(4 + iter, 4 + iter);
            iter++;

        } else if(spinning && !once){
            once = true; 
        }

        // Objects with texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID[0]);

        //SPHERE
        if (current_program == FULLCOLOR || current_program == FLATTEN)
        {
            GLint fragColorLocation = glGetUniformLocation(shaders[current_program].Program, "colorIn");
            glUniform3fv(fragColorLocation, 1, myColor);
        }

        sphereTransform.Transformation(
            glm::vec3(1.0f, 1.0f, 1.0f),
            0.0f, glm::vec3(0.0f, 1.0f, 0.0f),
            spherePosition,
            view);
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereTransform.modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereTransform.normalMatrix));
        sphereModel.Draw();
        
        // we activate the texture with id 1, and we bind the id to the loaded texture data
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureID[1]);

        // we pass the id of the texture (= to number X in GL_TEXTUREX at line 327) and the number of repetitions for the plane
        glUniform1i(textureLocation, 1);
        glUniform1f(repeatLocation, 80.0f);
        
        // PLANE
        if (current_program == FULLCOLOR || current_program == FLATTEN)
        {
            GLint fragColorLocation = glGetUniformLocation(shaders[current_program].Program, "colorIn");
            glUniform3fv(fragColorLocation, 1, planeColor);
        }

        planeTransform.Transformation(
            glm::vec3(2.5f, 1.0f, 2.5f),
            0.0f, glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, -10.0f, 0.0f),
            view);
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeTransform.modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeTransform.normalMatrix));
        planeModel.Draw();


        glfwSwapBuffers(window);
    }

    DeleteShaders();
    glfwTerminate();
    return 0;
}

int SetupOpenGL(){
    // Initialization of OpenGL context using GLFW
    glfwInit();
    // We set OpenGL specifications required for this application
    // In this case: 4.1 Core
    // It is possible to raise the values, in order to use functionalities of more recent OpenGL specs.
    // If not supported by your graphics HW, the context will not be created and the application will close
    // N.B.) creating GLAD code to load extensions, try to take into account the specifications and any extensions you want to use,
    // in relation also to the values indicated in these GLFW commands
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // we set if the window is resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // we create the application's window
    window = glfwCreateWindow(screenWidth, screenHeight, "Application", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // we put in relation the window and the callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // we disable the mouse cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // we define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // we enable Z test
    glEnable(GL_DEPTH_TEST);

    //the "clear" color for the frame buffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    return 1;
}

void SetupBlinPhong(){
    // we determine the position in the Shader Program of the uniform variables
    GLint matDiffuseLocation = glGetUniformLocation(shaders[current_program].Program, "diffuseColor");
    GLint matAmbientLocation = glGetUniformLocation(shaders[current_program].Program, "ambientColor");
    GLint matSpecularLocation = glGetUniformLocation(shaders[current_program].Program, "specularColor");
    GLint kaLocation = glGetUniformLocation(shaders[current_program].Program, "Ka");
    GLint kdLocation = glGetUniformLocation(shaders[current_program].Program, "Kd");
    GLint ksLocation = glGetUniformLocation(shaders[current_program].Program, "Ks");
    GLint shineLocation = glGetUniformLocation(shaders[current_program].Program, "shininess");
    GLint alphaLocation = glGetUniformLocation(shaders[current_program].Program, "alpha");
    GLint f0Location = glGetUniformLocation(shaders[current_program].Program, "F0");

    // we assign the value to the uniform variables
    glUniform3fv(matAmbientLocation, 1, ambientColor);
    glUniform3fv(matSpecularLocation, 1, specularColor);
    glUniform1f(shineLocation, shininess);
    glUniform1f(alphaLocation, alpha);
    glUniform1f(f0Location, F0);

    glUniform3fv(matDiffuseLocation, 1, diffuseColor);

    glUniform1f(ksLocation, Ka);
    glUniform1f(ksLocation, Kd);
    glUniform1f(ksLocation, Ks);
}

//-------------------------------------------------------------------------------------
// we create and compile shaders (code of Shader class is in include/utils/shader.h), and we add them to the list of available shaders
void SetupShaders()
{
    Shader shader1("00_basic.vert", "01_fullcolor.frag");
    shaders.push_back(shader1);
    Shader shader2("02_flatten.vert", "02_flatten.frag");
    shaders.push_back(shader2);
    Shader shader3("03_normal2color.vert", "03_normal2color.frag");
    shaders.push_back(shader3);
    Shader shader4("04_wave.vert", "04_wave.frag");
    shaders.push_back(shader4);
    Shader shader5("06_blinphong.vert", "06_blinphong.frag");
    shaders.push_back(shader5);
}

//---------------------------------------------------------------------------------
// we delete all the Shaders Programs
void DeleteShaders()
{
    for(GLuint i = 0; i < shaders.size(); i++)
        shaders[i].Delete();
}

//---------------------------------------------------------------------------------
// we print on console the name of the currently used shader
void PrintCurrentShader(int shader)
{
    std::cout << "Current shader:" << print_available_ShaderPrograms[shader]  << std::endl;

}

//////////////////////////////////////////
// we load the image from disk and we create an OpenGL texture
GLint LoadTexture(const char* path)
{
    GLuint textureImage;
    int w, h, channels;
    unsigned char* image;
    image = stbi_load(path, &w, &h, &channels, STBI_rgb);

    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;

    glGenTextures(1, &textureImage);
    glBindTexture(GL_TEXTURE_2D, textureImage);
    // 3 channels = RGB ; 4 channel = RGBA
    if (channels==3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    else if (channels==4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    // we set how to consider UVs outside [0,1] range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // we set the filtering for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);

    // we set the binding to 0 once we have finished
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureImage;
}


//---------------------------------------------------------------------------------
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // if P is pressed, we start/stop the animated rotation of models
    if(key == GLFW_KEY_P && action == GLFW_PRESS){
        spinning=!spinning;
        movingOnX = !movingOnX;
        instantiate = !instantiate;
    }

    // if P is pressed, we start/stop the animated rotation of models
    if(key == GLFW_KEY_R && action == GLFW_PRESS){
        R_KEY = true;
    }

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;

    // pressing a key between 1 and 5, we change the shader applied to the models
    if((key >= GLFW_KEY_1 && key <= GLFW_KEY_5) && action == GLFW_PRESS)
    {
        // "1" to "5" -> ASCII codes from 49 to 57
        // we subtract 48 (= ASCII CODE of "0") to have integers from 1 to 5
        // we subtract 1 to have indices from 0 to 4 in the shaders list
        current_program = (key-'0'-1);
        PrintCurrentShader(current_program);
    }

    // we keep trace of the pressed keys
    // with this method, we can manage 2 keys pressed at the same time:
    // many I/O managers often consider only 1 key pressed at the time (the first pressed, until it is released)
    // using a boolean array, we can then check and manage all the keys pressed at the same time
    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

//---------------------------------------------------------------------------------
// If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
void apply_camera_movements()
{
    // if a single WASD key is pressed, then we will apply the full value of velocity v in the corresponding direction.
    // However, if two keys are pressed together in order to move diagonally (W+D, W+A, S+D, S+A), 
    // then the camera will apply a compensation factor to the velocities applied in the single directions, 
    // in order to have the full v applied in the diagonal direction 

    GLboolean diagonal_movement = (keys[GLFW_KEY_W] || keys[GLFW_KEY_S]) && (keys[GLFW_KEY_A] || keys[GLFW_KEY_D]); 
    camera.SetMovementCompensation(diagonal_movement);
    
    if(keys[GLFW_KEY_W])
    {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if(keys[GLFW_KEY_S])
    {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if(keys[GLFW_KEY_A])
    {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if(keys[GLFW_KEY_D])
    {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

//---------------------------------------------------------------------------------
// callback for mouse events
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
      // we move the camera view following the mouse cursor
      // we calculate the offset of the mouse cursor from the position in the last frame
      // when rendering the first frame, we do not have a "previous state" for the mouse, so we set the previous state equal to the initial values (thus, the offset will be = 0)
      if(firstMouse)
      {
          lastX = xpos;
          lastY = ypos;
          firstMouse = false;
      }

      // offset of mouse cursor position
      GLfloat xoffset = xpos - lastX;
      GLfloat yoffset = lastY - ypos;

      // the new position will be the previous one for the next frame
      lastX = xpos;
      lastY = ypos;

      // we pass the offset to the Camera class instance in order to update the rendering
      camera.ProcessMouseMovement(xoffset, yoffset);

}
