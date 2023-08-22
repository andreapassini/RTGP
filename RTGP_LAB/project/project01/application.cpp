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
#include <string>

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
#include <utils/Scene.h>
#include <colliders/PlaneCollider.h>
#include <colliders/sphereCollider.h>
#include <colliders/CapsuleCollider.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <utils/transform.h>
#include <physicsSimulation/physicsSimulation.h>
#include <colliders/sphereCollider.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


// we include the library for images loading
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image/stb_image.h"
#define stringify( name ) #name

#define UP_DIRECTION glm::vec3(0.0f, 1.0f, 0.0f)
#define DOWN_DIRECTION glm::vec3(0.0f, -1.0f, 0.0f)

#define FORWARD_DIRECTION glm::vec3(0.0f, 0.0f, 1.0f)
#define BACKWARD_DIRECTION glm::vec3(0.0f, 0.0f, -1.0f)

#define RIGHT_DIRECTION glm::vec3(1.0f, 0.0f, 0.0f)
#define LEFT_DIRECTION glm::vec3(-1.0f, 0.0f, 0.0f)

GLFWwindow* window;
GLuint screenWidth = 1200, screenHeight = 900;

int SetupOpenGL();
void DebugLogStatus();

void RenderScene1(Shader &shader, glm::mat4 projection, glm::mat4 view, Transform &planeTransform, Model &planeModel, Transform &sphereTransform, Model &sphereModel, Transform &sphereTra1, Model &sphereModel1);
void RenderScene(Shader &shader, Scene &scene, glm::mat4 projection, glm::mat4 view);

GLint LoadTexture(const char* path);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void apply_camera_movements();
void imGuiSetup(GLFWwindow *window);
void PrintVec3(glm::vec3* vec);
void MoveSphere(glm::vec3 direction, int action);
void RotateSphere(float angle, int action);
void ForceBlinnPhongShaderSetup(Shader forceBlinnPhongShader, Transform clothTransform, glm::mat4 projection, glm::mat4 view);
void ForceGGXShaderSetup(Shader forceGGXShader, Transform clothTransform, glm::mat4 projection, glm::mat4 view);
void SetUpClothShader(Shader shader, Transform clothTransform, glm::mat4 projection, glm::mat4 view);

bool keys[1024];
bool R_KEY = false;
    GLfloat lastX, lastY;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLfloat orientationY = 0.0f;
GLfloat spin_speed = 30.0f;
GLboolean pKeyPressed = GL_TRUE;
GLfloat positionZ = 0.0f;
GLfloat movement_speed = 5.0f;
GLboolean movingOnX = GL_TRUE;
GLboolean instantiate = GL_FALSE;

GLboolean wireframe = GL_FALSE;

Camera camera(glm::vec3(0.0f, -2.0f, 7.0f), GL_FALSE);

glm::vec3 lightPosition = glm::vec3(1.0f, 1.0f, 1.0f);

// weights for the diffusive, specular and ambient components
GLfloat Kd = 3.0f;
// weights for the diffusive, specular    and ambient components
GLfloat Ka_Sphere = 0.5f;
GLfloat Kd_Sphere = 0.5f;
GLfloat Ks_Sphere = 0.5f;
GLfloat shininess = 0.5f;

glm::vec3 ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f);

int shaderNumber = 0;

GLfloat Kd_GGXSphere = 3.0f;

// roughness index for GGX shader  
GLfloat alpha = 0.2f;
// roughness index for GGX shader
GLfloat alpha_Sphere = 0.35f;
// Fresnel reflectance at 0 degree (Schlik's approximation)
GLfloat F0 = 0.9f;
// Fresnel reflectance at 0 degree (Schlik's approximation)
GLfloat F0_Sphere = 0.9f;

vector<GLint> textureID;
GLfloat repeat = 1.0f;  // UV repetitions


GLfloat myColor[] = {1.0f,0.0f,0.0f};
GLfloat clothColor[] = {0.0f, 31.0f, 0.0f};
GLfloat coral[] = {1.0f, 0.5f, 0.31f};
GLfloat planeColor[] = {0.13f, 0.07f, 0.34f};


glm::vec3 startingPosition(0.0f, 0.0f, 0.0f);

bool once = true;
unsigned int iter = 0;

int clothDim = 30;
float particleOffset = 0.15f;
bool pinned = true;
ConstraintType springType = POSITIONAL;
float gravity = -9.8f;
float mass = 0.1f;
float K = 0.5f;
float U = 0.1f;
int constraintIterations = 10;
int constraintLevel = 1;
int collisionIterations = 10;

unsigned int windowSize = 10;
unsigned int overlap = 3;

glm::vec3 spherePosition(0.0f, 2.0f, 0.0f);
glm::vec3 spherePosition1(0.0f, 1.0f, 0.0f);
glm::vec3 spherePosition2(0.0f, 0.0f, 0.0f);


glm::vec3 cubePosition(3.0f, -4.5f, -2.5f);
glm::vec3 planePosition(0.0f, -10.0f, 0.0f);

PerformanceCalculator performanceCalculator(windowSize, overlap);

bool pausePhysics;

float sphereMinSpeed = 1.0f;
float sphereMaxSpeed = 15.0f;
float sphereSpeed = sphereMinSpeed;
float sphereAccel = 3.0f;

float sphereMinAngularVelocity = 25.0f;
float sphereMaxAngularVelocity = 150.0f;
float sphereAngularVelocity = 0.0f;
float sphereAngularAccel = 10.0f;

float sphereRotation = 0.0f;

Transform sphereTransform1;
Transform sphereTransform2;

Cloth* c;

Scene scene1;
Scene scene2;

Transform plane1_TransformScene2;


int main()
{
    if(SetupOpenGL() == -1)
        return -1;

    std::cout << "SetupOpenGL: complete" << std::endl;

    Shader illumination_shader = Shader("06_illumination.vert", "06_illumination.frag");
    Shader force_shader = Shader("07_force.vert", "07_force.frag");
    Shader fullColor_shader = Shader("00_basic.vert", "01_fullcolor.frag");
    Shader normal_shader = Shader("03_normal2color.vert", "03_normal2color.frag");
    Shader force_BlinnPhong_shader = Shader("08_force_Phong.vert", "08_force_Phong.frag");

    std::cout << "Shader Set-Up: complete" << std::endl;

    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    // View matrix (=camera): position, view direction, camera "up" vector
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    Model sphereModel("../../models/sphere.obj");
    Model sphereModel1("../../models/sphere.obj");
    Model sphereModel2("../../models/sphere.obj");

    Model planeModel("../../models/plane.obj");
    Model cubeModel("../../models/cube.obj");

    std::cout << "Model load: complete" << std::endl;

    textureID.push_back(LoadTexture("../../textures/UV_Grid_Sm.png"));
    textureID.push_back(LoadTexture("../../textures/SoilCracked.png"));

    std::cout << "Texture load: complete" << std::endl;


    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    sphereTransform1 = Transform(view);
    sphereTransform2 = Transform(view);

    sphereTransform1.scale = 1.0f;
    sphereTransform1.translation = glm::vec3(-3.0f, 2.0f, 0.0f);
    // sphereTransform1.rotation = &Quaternion();

    sphereTransform2.scale = 1.0f;
    sphereTransform2.translation = glm::vec3(-3.0f, 0.0f, 0.0f);
    // sphereTransform2.rotation = &Quaternion();

    Transform planeTransform(view);
    planeTransform.translation = planePosition; 

    std::cout << "Spheres and Planes Transform: complete" << std::endl;

    Transform clothTransform(view);
    Cloth cloth(clothDim, particleOffset, startingPosition, &clothTransform, pinned, springType, K, U, constraintIterations, gravity, mass, collisionIterations, constraintLevel);
    
    std::cout << "Cloth Transform: complete" << std::endl;

    c = &cloth;

    PerformanceCalculator performanceCalculator(windowSize, overlap);

    int type = 0;   // Used in ImGui display
    
    PhysicsSimulation physicsSimulation;
    float currentTime = glfwGetTime();
    physicsSimulation.StartPhysicsSimulation(currentTime);

    //physicsSimulation.AddObjectToPhysicWorld(&sphereTransform, 1.0f, false);

    pausePhysics = false;

    PlaneCollider planeCollider(&planeTransform);

    SphereCollider sphereCollider1(&sphereTransform1, sphereTransform1.scale);
    SphereCollider sphereCollider2(&sphereTransform2, sphereTransform2.scale);

    // scene1.planes.push_back(&planeCollider);
    scene1.spheres.push_back(&sphereCollider1);
    scene1.spheres.push_back(&sphereCollider2);
    //scene.capsules.push_back(&capsuleCollider);
    
    std::cout << "Colliders: complete" << std::endl;

    GameObject* sphere1 = new GameObject(&sphereTransform1, &sphereModel1);
    TextureParameter* sphereTextureParameter1 = new TextureParameter(true, 0, repeat);
    RenderableObject* renderableSphere1 = new RenderableObject(sphere1, sphereTextureParameter1);
    scene1.renderableObjects.push_back(renderableSphere1);

    GameObject* sphere2 = new GameObject(&sphereTransform2, &sphereModel1);
    TextureParameter* sphereTextureParameter2 = new TextureParameter(true, 0, repeat);
    RenderableObject* renderableSphere2 = new RenderableObject(sphere2, sphereTextureParameter2);
    scene1.renderableObjects.push_back(renderableSphere2);


    std::cout << "Scene 1: complete" << std::endl;


    plane1_TransformScene2.scale = 2.0f;
    plane1_TransformScene2.translation = glm::vec3(0.0f, -6.0f, -5.0f); 
    plane1_TransformScene2.rotation = &Quaternion(glm::vec3(1.0f, 0.0f, 0.0f), 37.0f);
    GameObject* plane1_GOScene2 = new GameObject(&plane1_TransformScene2, &planeModel);
    TextureParameter* plane1_TP = new TextureParameter(true, 1, 80.0f);
    RenderableObject* renderablePlane1 = new RenderableObject(plane1_GOScene2, plane1_TP);
    scene2.renderableObjects.push_back(renderablePlane1);

    // GameObject* spherePlane_GOScene2 = new GameObject(&plane1_TransformScene2, &sphereModel);
    // TextureParameter* spherePlane_TP = new TextureParameter(true, 0, repeat);
    // RenderableObject* renderableSpherePlane1 = new RenderableObject(spherePlane_GOScene2, spherePlane_TP);
    // scene2.renderableObjects.push_back(renderableSpherePlane1);

    std::cout << "Scene 2: complete" << std::endl;


    // Rendering loop: this code is executed at each frame
    while(!glfwWindowShouldClose(window))
    {
        // we determine the time passed from the beginning
        // and we calculate time difference between current frame rendering and the previous one
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        currentTime = glfwGetTime();

        performanceCalculator.Step(deltaTime);

        glfwPollEvents();
        apply_camera_movements();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // render your GUI
        ImGui::Begin("Controls");

        ImGui::Text("Cloth simulation:");
        ImGui::NewLine;
        ImGui::Text("Framerate (ms): %d", (int)performanceCalculator.framerate);
        ImGui::NewLine;
        ImGui::Text("Press P to Update Cloth");
        ImGui::NewLine;
        ImGui::SliderInt("Grid dim", &clothDim, 10, 100);
        ImGui::NewLine;
        ImGui::SliderFloat("Particle offset", &particleOffset, 0.05f, 1.0f);
        ImGui::NewLine;
        ImGui::SliderInt("Force Shader to use", &shaderNumber, 0, 2);
        if(shaderNumber == 0){
            ImGui::Text("BlinnPhong");
        } else if(shaderNumber == 1) {
            ImGui::Text("GGX");
        } else if(shaderNumber == 2) {
            ImGui::Text("Normal");
        }

        ImGui::NewLine;
        ImGui::Text("Physic Simulation");
        ImGui::NewLine;
        ImGui::SliderFloat("Gravity", &gravity, -0.0f, -9.8f);
        ImGui::NewLine;
        ImGui::SliderFloat("Mass", &mass, 0.0f, 2.0f);

        ImGui::NewLine;
        ImGui::Text("Constraints");
        ImGui::NewLine;
        if(ImGui::SliderInt("type", &type, 0, 3)){
            switch(type)
            {
            case 0:
                //springType = POSITIONAL;
                K = 0.5f;
                gravity = -9.8f;
                constraintIterations = 10;
                break;
            case 1:
                // springType = PHYSICAL;
                K = 15.0f;
                gravity = -9.8f;
                constraintIterations = 5;
                constraintLevel = 2;
                break;
            case 2:
                // springType = POSITIONAL_ADVANCED;
                K = 0.5f;
                gravity = -9.8f;
                constraintIterations = 15;
                break;
            case 3:
                // springType = PHYSICAL_ADVANCED;
                K = 3.5f;
                gravity = -9.8f;
                constraintIterations = 1;
                break;
            default:
                break;
            }
        }
        ImGui::SameLine();
        switch (type)
        {
        case 0:
            springType = POSITIONAL;
            ImGui::Text("POSITIONAL");
            break;
        case 1:
            springType = PHYSICAL;
            ImGui::Text("PHYSICAL");
            break;
        case 2:
            springType = POSITIONAL_ADVANCED;
            ImGui::Text("POSITIONAL_ADVANCED");

            ImGui::NewLine;
            ImGui::SliderFloat("U", &U, 0.00f, 2.0f);

            break;
        case 3:
            springType = PHYSICAL_ADVANCED;
            ImGui::Text("PHYSICAL_ADVANCED");

            ImGui::NewLine;
            ImGui::SliderFloat("U", &U, 0.00f, 2.0f);

            break;
        default:
            break;
        }

        // float K = 0.5f;
        ImGui::NewLine;
        ImGui::SliderFloat("K", &K, 0.01f, 25.0f);

        ImGui::NewLine;
        ImGui::SliderInt("Constraint Iterations", &constraintIterations, 0, 25);

        ImGui::NewLine;
        ImGui::SliderInt("Constraint Level", &constraintLevel, 1, 5);

        ImGui::NewLine;
        ImGui::Text("Collisions");
        ImGui::NewLine;
        ImGui::SliderInt("collisions Iterations", &collisionIterations, 0, 25);

        ImGui::End();


        // View matrix (=camera): position, view direction, camera "up" vector
        view = camera.GetViewMatrix();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


        // CLOTH        
        clothTransform.Transformation(
            glm::vec3(1.0f, 1.0f, 1.0f),
            0.0f, glm::vec3(0.0f, 1.0f, 0.0f),
            startingPosition,
            view
        );

        unsigned int maxIter = 4U;
        unsigned int physIter = 0U;

        if(!pausePhysics){
            while(!physicsSimulation.isPaused &&  currentTime > physicsSimulation.getVirtualTIme()){
                cloth.ResetShaderForce();
                physicsSimulation.AddForceToAll(glm::vec3(0.0f, gravity, 0.0f));
                cloth.AddGravityForce();
                physicsSimulation.FixedTimeStep(currentTime);
                //cloth.PhysicsSteps(deltaTime, (glm::vec4(spherePosition, 1.0f) * sphereTransform.modelMatrix), 1.0f, planePosition.y + 0.1f);
                //cloth.PhysicsSteps((glm::vec4(spherePosition, 1.0f) * sphereTransform.modelMatrix), 1.0f, planePosition.y + 0.1f);
                cloth.PhysicsSteps(&scene1);
                physIter++;

                if(physIter > maxIter){
                    std::cout << "Physics Simulation lagging " << std::endl;
                    physicsSimulation.SynchVirtualTime(currentTime);
                    break;
                }
            }
        
        }

        if(shaderNumber == 0){
            force_BlinnPhong_shader.Use();
            ForceBlinnPhongShaderSetup(force_BlinnPhong_shader, clothTransform, projection, view);

        } else if(shaderNumber == 1) {
            force_shader.Use();
            ForceGGXShaderSetup(force_shader, clothTransform, projection, view);
            
        } else if(shaderNumber == 2) {
            normal_shader.Use();
            SetUpClothShader(normal_shader, clothTransform, projection, view);
        }


        cloth.Draw();
        
        if(!pKeyPressed && once)
        {
            cloth.~Cloth();
            pinned = !pinned;
            new(&cloth) Cloth(clothDim, particleOffset, startingPosition, &clothTransform, pinned, springType, K, U, constraintIterations, gravity, mass, collisionIterations, constraintLevel);
            once = false;
            //DebugLogStatus();
            //cloth.CutAHole(4 + iter, 4 + iter);
            iter++;
        } else if(pKeyPressed && !once){
            once = true; 
        }

        RenderScene(illumination_shader, scene1, projection, view);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();


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
    //glEnable(GL_CULL_FACE);

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
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    glClearColor(0.26f, 0.46f, 0.98f, 1.0f);

    imGuiSetup(window);

    return 1;
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
        pKeyPressed=!pKeyPressed;
        movingOnX = !movingOnX;
        instantiate = !instantiate;
    }

    // // pressing a key between 1 and 5, we change the shader applied to the models
    // if((key >= GLFW_KEY_1 && key <= GLFW_KEY_5) && action == GLFW_PRESS)
    // {
    //     // "1" to "5" -> ASCII codes from 49 to 57
    //     // we subtract 48 (= ASCII CODE of "0") to have integers from 1 to 5
    //     // we subtract 1 to have indices from 0 to 4 in the shaders list
    //     unsigned int pressedInt = (key-'0'-1);
    // }

    // if R is pressed, we start/stop the animated rotation of models
    if(key == GLFW_KEY_R && action == GLFW_PRESS){
        R_KEY = true;
        pausePhysics = !pausePhysics;
    }

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;

    if(key == GLFW_KEY_UP){
        MoveSphere(glm::vec3(camera.Front.x, 0.0f, camera.Front.z), action);
    } 
    if(key == GLFW_KEY_DOWN){
        MoveSphere(-glm::vec3(camera.Front.x, 0.0f, camera.Front.z), action);
    } 
    if(key == GLFW_KEY_LEFT){
        MoveSphere(-camera.Right, action);
    } 
    if(key == GLFW_KEY_RIGHT){
        MoveSphere(camera.Right, action);
    } 
    if(key == GLFW_KEY_SPACE){
        MoveSphere(camera.WorldUp, action);
    } 
    if(key == GLFW_KEY_LEFT_CONTROL){
        MoveSphere(-camera.WorldUp, action);
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

// Scene rendering
void RenderScene(Shader &shader, Scene &scene, glm::mat4 projection, glm::mat4 view){
    shader.Use();

    // we pass projection and view matrices to the Shader Program
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

    // we determine the position in the Shader Program of the uniform variables
    GLint lightDirLocation = glGetUniformLocation(shader.Program, "lightVector");
    GLint kdLocation = glGetUniformLocation(shader.Program, "Kd");
    GLint alphaLocation = glGetUniformLocation(shader.Program, "alpha");
    GLint f0Location = glGetUniformLocation(shader.Program, "F0");

    // we assign the value to the uniform variables
    glUniform3fv(lightDirLocation, 1, glm::value_ptr(lightPosition));
    glUniform1f(kdLocation, Kd);
    glUniform1f(alphaLocation, alpha);
    glUniform1f(f0Location, F0);

    // we pass the needed uniforms
    GLint textureLocation = glGetUniformLocation(shader.Program, "tex");
    GLint repeatLocation = glGetUniformLocation(shader.Program, "repeat");

    
    int size = scene.renderableObjects.size();

    for(int i=0; i < size; i++){
        // Objects with texture
        if(scene.renderableObjects[i]->textureParameter->useTexture){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID[scene.renderableObjects[i]->textureParameter->textureId]);
            glUniform1i(textureLocation, 0);
            glUniform1f(repeatLocation, scene.renderableObjects[i]->textureParameter->repeat);
        }

        scene.renderableObjects[i]->gameObject->transform->Transformation(view);

        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(scene.renderableObjects[i]->gameObject->transform->modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(scene.renderableObjects[i]->gameObject->transform->normalMatrix));

        scene.renderableObjects[i]->gameObject->model->Draw();
    }

}
void RenderScene1(Shader &shader, glm::mat4 projection, glm::mat4 view, Transform &planeTransform, Model &planeModel, Transform &sphereTransform, Model &sphereModel, Transform &sphereTra1, Model &sphereModel1){

    shader.Use();

    // we pass projection and view matrices to the Shader Program
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

    // we determine the position in the Shader Program of the uniform variables
    GLint lightDirLocation = glGetUniformLocation(shader.Program, "lightVector");
    GLint kdLocation = glGetUniformLocation(shader.Program, "Kd");
    GLint alphaLocation = glGetUniformLocation(shader.Program, "alpha");
    GLint f0Location = glGetUniformLocation(shader.Program, "F0");

    // we assign the value to the uniform variables
    glUniform3fv(lightDirLocation, 1, glm::value_ptr(lightPosition));
    glUniform1f(kdLocation, Kd);
    glUniform1f(alphaLocation, alpha);
    glUniform1f(f0Location, F0);

    // we pass the needed uniforms
    GLint textureLocation = glGetUniformLocation(shader.Program, "tex");
    GLint repeatLocation = glGetUniformLocation(shader.Program, "repeat");

    // OBJECTS
    // Objects with texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID[0]);
    glUniform1i(textureLocation, 0);
    glUniform1f(repeatLocation, repeat);

    // Sphere
    sphereTransform.Transformation(
        glm::vec3(1.0f),
        sphereRotation, glm::vec3(0.0f, 1.0f, 0.0f),
        spherePosition,
        view);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereTransform.modelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereTransform.normalMatrix));
    sphereModel.Draw();
    sphereTransform.PrintTransform();

    // Sphere
    sphereTra1.Transformation(
        glm::vec3(1.0f),
        sphereRotation, glm::vec3(0.0f, 1.0f, 0.0f),
        spherePosition1,
        view);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereTra1.modelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereTra1.normalMatrix));
    sphereModel1.Draw();
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureID[1]);
    glUniform1i(textureLocation, 1);
    glUniform1f(repeatLocation, 80.0);

    
    // PLANE
    planeTransform.Transformation(
        glm::vec3(10.0f, 1.0f, 10.0f),
        0.0f, glm::vec3(0.0f, 1.0f, 0.0f),
        planePosition,
        view);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeTransform.modelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeTransform.normalMatrix));
    planeModel.Draw();
}

void imGuiSetup(GLFWwindow *window)
{
    // ImGui SETUP
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}
void DebugLogStatus(){
    std::cout << "Framerate: " << (int)performanceCalculator.framerate << std::endl;
    std::cout << std::endl;
    std::cout << "Cloth: " << std::endl;
    std::cout << "  - Spring Type: " << springType << std::endl;
    std::cout << "  - K: " << K << std::endl;
    std::cout << "  - Constraint Iterations: " << constraintIterations << std::endl;
}
void PrintVec3(glm::vec3* vec){

    std::cout << vec->x << ", " << vec->y << ", " << vec->z << std::endl;
}
void MoveSphere(glm::vec3 direction, int action){
    if(action == GLFW_PRESS){
        sphereSpeed = sphereMinSpeed;
    }
    if(action == GLFW_REPEAT){
        sphereSpeed += sphereSpeed * sphereAccel * deltaTime;
        if(sphereSpeed > sphereMaxSpeed){
            sphereSpeed = sphereMaxSpeed;
        }
    }

    direction *= sphereSpeed * deltaTime;

    sphereTransform1.translation += direction;
    // std::cout << "Move" << std::endl;
    // sphereTransform1.PrintTransform();
    sphereTransform2.translation += direction;

    plane1_TransformScene2.translation += direction;

    // plane1_TransformScene2.rotation->real = cos((plane1_TransformScene2.rotation->GetAngleDegree() + 0.1f) / 2);
    // std::cout << "Print angle before adding: " << plane1_TransformScene2.rotation->GetAngleDegree() << std::endl;
    // plane1_TransformScene2.rotation = &Quaternion(glm::vec3(0.0f, 1.0f, 0.0f), plane1_TransformScene2.rotation->GetAngleDegree() + 0.01f);

    // plane1_TransformScene2.PrintTransform();

    // Move pinned particles
    // for(int i = 0; i < clothDim; i++){
    //     for(int j = 0; j < clothDim; j++){
    //         Particle* p = (*c).getParticle(i, j, clothDim);
    //         if(p->movable == false){
    //             p->pos += direction;
    //         }
    //     }
    // }
}
void RotateSphere(float angle, int action){
    if(action == GLFW_PRESS){
        sphereAngularVelocity = sphereMinAngularVelocity;
    }
    if(action == GLFW_REPEAT){
        sphereAngularVelocity += sphereAngularVelocity * sphereAngularAccel * deltaTime;
        if(sphereAngularVelocity > sphereMaxAngularVelocity){
            sphereAngularVelocity = sphereMaxAngularVelocity;
        }
    }
    std::cout << "Rot: " << sphereAngularVelocity << std::endl;
    sphereRotation += angle * sphereAngularVelocity * deltaTime;
}
void ForceBlinnPhongShaderSetup(Shader forceBlinnPhongShader, Transform clothTransform, glm::mat4 projection, glm::mat4 view){

    SetUpClothShader(forceBlinnPhongShader, clothTransform, projection, view);

    GLint lightDirLocation = glGetUniformLocation(forceBlinnPhongShader.Program, "light");
    GLint ambientColorLocation = glGetUniformLocation(forceBlinnPhongShader.Program, "ambientColor");
    GLint specularColorLocation = glGetUniformLocation(forceBlinnPhongShader.Program, "specularColor");
    GLint kaLocation = glGetUniformLocation(forceBlinnPhongShader.Program, "Ka");
    GLint kdLocation = glGetUniformLocation(forceBlinnPhongShader.Program, "Kd");
    GLint ksLocation = glGetUniformLocation(forceBlinnPhongShader.Program, "Ks");
    GLint shininessLocation = glGetUniformLocation(forceBlinnPhongShader.Program, "shininess");

    // we assign the value to the uniform variables
    glUniform3fv(lightDirLocation, 1, glm::value_ptr(lightPosition));
    glUniform3fv(ambientColorLocation, 1, glm::value_ptr(ambientColor));
    glUniform3fv(specularColorLocation, 1, glm::value_ptr(specularColor));
    glUniform1f(kaLocation, Ka_Sphere);
    glUniform1f(kdLocation, Kd_Sphere);
    glUniform1f(ksLocation, Ks_Sphere);
    glUniform1f(shininessLocation, shininess);

}
void ForceGGXShaderSetup(Shader forceGGXShader, Transform clothTransform, glm::mat4 projection, glm::mat4 view){

    SetUpClothShader(forceGGXShader, clothTransform, projection, view);

    GLint lightDirLocation = glGetUniformLocation(forceGGXShader.Program, "lightVector");
    GLint kdLocation = glGetUniformLocation(forceGGXShader.Program, "Kd");
    GLint alphaLocation = glGetUniformLocation(forceGGXShader.Program, "alpha");
    GLint f0Location = glGetUniformLocation(forceGGXShader.Program, "F0");
    
    glUniform3fv(lightDirLocation, 1, glm::value_ptr(lightPosition));
    glUniform1f(kdLocation, Kd_GGXSphere);
    glUniform1f(alphaLocation, alpha_Sphere);
    glUniform1f(f0Location, F0_Sphere);

}
void SetUpClothShader(Shader shader, Transform clothTransform, glm::mat4 projection, glm::mat4 view){
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(clothTransform.modelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(clothTransform.normalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
}