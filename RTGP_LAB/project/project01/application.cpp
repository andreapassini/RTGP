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
void MoveSphere(Transform* sphere_transform , glm::vec3 direction, int action);
void MoveSphereAndCloth(Transform* sphere_transform , glm::vec3 direction, int action);
void RotateSphere(float angle, int action);
void ForceBlinnPhongShaderSetup(Shader forceBlinnPhongShader, Transform clothTransform, glm::mat4 projection, glm::mat4 view);
void ForceGGXShaderSetup(Shader forceGGXShader, Transform clothTransform, glm::mat4 projection, glm::mat4 view);
void ColorGGXShaderSetup(Shader colorPhongShader, Transform clothTransform, glm::mat4 projection, glm::mat4 view);
void SetUpClothShader(Shader shader, Transform clothTransform, glm::mat4 projection, glm::mat4 view);
void UpdateScene1 (Scene* scene);
void UpdateScene2 (Scene* scene);
void UpdateScene3 (Scene* scene);
void MoveClothPinnedParticles(glm::vec3 direction);
void ChangeScene(Scene* sceneToChange);
void Start1(Scene* scene);
void Start2(Scene* scene);
void Start3(Scene* scene);

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
float cuttingDistanceMultiplier = 5.0f;

unsigned int windowSize = 10;
unsigned int overlap = 3;

glm::vec3 spherePosition(0.0f, 2.0f, 0.0f);
glm::vec3 spherePosition1(0.0f, 1.0f, 0.0f);
glm::vec3 spherePosition2(0.0f, 0.0f, 0.0f);


glm::vec3 cubePosition(3.0f, -4.5f, -2.5f);
glm::vec3 planePosition(0.0f, -10.0f, 0.0f);

PerformanceCalculator performanceCalculator(windowSize, overlap);

bool pausePhysics;

float sphereMinSpeed = 2.0f;
float sphereMaxSpeed = 15.0f;
float sphereSpeed = sphereMinSpeed;
float sphereAccel = 3.0f;

float sphereMinAngularVelocity = 25.0f;
float sphereMaxAngularVelocity = 150.0f;
float sphereAngularVelocity = 0.0f;
float sphereAngularAccel = 10.0f;

float sphereRotation = 0.0f;


Cloth* c;

Scene* activeScene;
Scene* previousActiveScene;

glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);

int action = 0;


int main()
{
    if(SetupOpenGL() == -1)
        return -1;

    std::cout << "SetupOpenGL: complete" << std::endl;

    Shader illumination_shader = Shader("06_illumination.vert", "06_illumination.frag");
    Shader force_shader = Shader("07_force.vert", "07_force.frag");
    Shader fullColor_shader = Shader("00_basic.vert", "01_fullcolor.frag");
    Shader normal_shader = Shader("03_normal2color.vert", "03_normal2color.frag");
    Shader color_shader = Shader("09_color_GGX.vert", "09_color_GGX.frag");
    Shader force_BlinnPhong_shader = Shader("08_force_Phong.vert", "08_force_Phong.frag");

    std::cout << "Shader Set-Up: complete" << std::endl;

    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    // View matrix (=camera): position, view direction, camera "up" vector
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    Scene scene1;
    Scene scene2;
    Scene scene3;

    std::vector<Scene*> scenes;

    Model sphereModel("../../models/sphere.obj");
    Model sphereModel1("../../models/sphere.obj");
    Model sphereModel2("../../models/sphere.obj");

    Model planeModel("../../models/plane.obj");
    Model cubeModel("../../models/cube.obj");

    std::cout << "Model load: complete" << std::endl;

    textureID.push_back(LoadTexture("../../textures/UV_Grid_Sm.png"));
    textureID.push_back(LoadTexture("../../textures/SoilCracked.png"));

    std::cout << "Texture load: complete" << std::endl;

    Transform clothTransform(view);
    Cloth cloth(clothDim, particleOffset, startingPosition, &clothTransform, pinned, springType, K, U, constraintIterations, gravity, mass, collisionIterations, constraintLevel, cuttingDistanceMultiplier);
    
    std::cout << "Cloth Transform: complete" << std::endl;

    c = &cloth;

    PerformanceCalculator performanceCalculator(windowSize, overlap);

    int type = 0;   // Used in ImGui display
    
    PhysicsSimulation physicsSimulation;
    float currentTime = glfwGetTime();
    physicsSimulation.StartPhysicsSimulation(currentTime);

    //physicsSimulation.AddObjectToPhysicWorld(&sphereTransform, 1.0f, false);

    pausePhysics = false;


    // -----------------------------------------------------------------------------------
    // Scene 1
    std::cout << "Scene 1: Loading... " << std::endl;

    // Sphere 1
    Transform sphereTransform1;
    sphereTransform1 = Transform(view);
    // sphereTransform1.scale = 1.0f;
    sphereTransform1.translation = glm::vec3(0.0f, 0.0f, 0.0f);
    // sphereTransform1.rotation = &Quaternion();

    GameObject* sphere1 = new GameObject(&sphereTransform1, &sphereModel);
    TextureParameter* sphereTextureParameter1 = new TextureParameter(true, 0, repeat);
    RenderableObject* renderableSphere1 = new RenderableObject(sphere1, sphereTextureParameter1);
    scene1.renderableObjects.push_back(renderableSphere1);

    SphereCollider sphereCollider1(&sphereTransform1, sphereTransform1.scale);
    scene1.spheres.push_back(&sphereCollider1);

    // Plane 1
    Transform plane1_TransformScene1;
    plane1_TransformScene1 = Transform(view);
    plane1_TransformScene1.scale = 2.0f;
    plane1_TransformScene1.translation = glm::vec3(0.0f, -6.0f, -5.0f); 
    plane1_TransformScene1.rotation = &glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    GameObject* plane1_GOScene1 = new GameObject(&plane1_TransformScene1, &planeModel);
    TextureParameter* plane1_TP = new TextureParameter(true, 1, 80.0f);
    RenderableObject* renderablePlane1 = new RenderableObject(plane1_GOScene1, plane1_TP);
    scene1.renderableObjects.push_back(renderablePlane1);

    PlaneCollider planeCollider(&plane1_TransformScene1, glm::rotate(*plane1_TransformScene1.rotation, planeModel.meshes[0].vertices[0].Normal));

    scene1.planes.push_back(&planeCollider);

    scene1.Start = Start1;
    scene1.Update = UpdateScene1;
    scenes.push_back(&scene1);

    std::cout << "Scene 1: loading complete" << std::endl;

    // -----------------------------------------------------------------------------
    // Scene 2
    std::cout << "Scene 2: Loading... " << std::endl;

    Transform sphere3_transform;
    sphere3_transform = Transform(view);
    sphere3_transform.scale = 1.0f;
    sphere3_transform.translation = glm::vec3(2.0f, -0.05f, -0.5f);
    sphere3_transform.rotation = &glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    GameObject* sphere3 = new GameObject(&sphere3_transform, &sphereModel);
    TextureParameter* sphereTextureParameter3 = new TextureParameter(true, 0, repeat);
    RenderableObject* renderableSphere3 = new RenderableObject(sphere3, sphereTextureParameter3);
    scene2.renderableObjects.push_back(renderableSphere3);

    SphereCollider sphereCollider3(&sphere3_transform, sphere3_transform.scale);
    scene2.spheres.push_back(&sphereCollider3);

    Transform sphere31_transform;
    sphere31_transform = Transform(view);
    sphere31_transform.scale = 1.0f;
    sphere31_transform.translation = glm::vec3(2.0f, -1.55f, -0.5f);
    sphere31_transform.rotation = &glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    GameObject* sphere31 = new GameObject(&sphere31_transform, &sphereModel);
    TextureParameter* sphereTextureParameter31 = new TextureParameter(true, 0, repeat);
    RenderableObject* renderableSphere31 = new RenderableObject(sphere31, sphereTextureParameter31);
    scene2.renderableObjects.push_back(renderableSphere31);

    SphereCollider sphereCollider31(&sphere31_transform, sphere31_transform.scale);
    scene2.spheres.push_back(&sphereCollider31);


    Transform plane1_Transform1_scene2;
    plane1_Transform1_scene2 = Transform(view),
    plane1_Transform1_scene2.translation = glm::vec3(7.0f, -5.0f, 0.0f);
    plane1_Transform1_scene2.scale = 10.0f;
    plane1_Transform1_scene2.rotation = &glm::angleAxis(glm::radians(25.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    GameObject* plane1_GO_scene2 = new GameObject(&plane1_Transform1_scene2, &planeModel);
    TextureParameter* plane1_TP_scene2 = new TextureParameter(true, 1, 80.0f);
    RenderableObject* renderablePlane1_scene2 = new RenderableObject(plane1_GO_scene2, plane1_TP_scene2);
    scene2.renderableObjects.push_back(renderablePlane1_scene2);

    PlaneCollider plane1_collider_scene2(&plane1_Transform1_scene2, glm::rotate(*plane1_Transform1_scene2.rotation, plane1_GO_scene2->model->meshes[0].vertices[0].Normal));
    scene2.planes.push_back(&plane1_collider_scene2);

    Transform plane2_Transform1_scene2;
    plane2_Transform1_scene2 = Transform(view),
    plane2_Transform1_scene2.translation = glm::vec3(-7.0f, -5.0f, 0.0f);
    plane2_Transform1_scene2.scale = 10.0f;
    plane2_Transform1_scene2.rotation = &glm::angleAxis(glm::radians(-25.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    GameObject* plane2_GO_scene2 = new GameObject(&plane2_Transform1_scene2, &planeModel);
    TextureParameter* plane2_TP_scene2 = new TextureParameter(true, 1, 80.0f);
    RenderableObject* renderablePlane2_scene2 = new RenderableObject(plane2_GO_scene2, plane2_TP_scene2);
    scene2.renderableObjects.push_back(renderablePlane2_scene2);

    PlaneCollider plane2_collider_scene2(&plane2_Transform1_scene2, glm::rotate(*plane2_Transform1_scene2.rotation, plane2_GO_scene2->model->meshes[0].vertices[0].Normal));
    scene2.planes.push_back(&plane2_collider_scene2);

    scene2.Start = Start2;
    scene2.Update = UpdateScene2;

    scenes.push_back(&scene2);

    std::cout << "Scene 2: loading complete" << std::endl;



    std::cout << "Scene 3: Loading... " << std::endl;
    // Scene 3
    
    Transform sphere4_transform;
    sphere4_transform = Transform(view);
    sphere4_transform.scale = 0.5f;
    sphere4_transform.translation = glm::vec3(2.0f, -3.0f, -3.0f);
    sphere4_transform.rotation = &glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    GameObject* sphere4 = new GameObject(&sphere4_transform, &sphereModel);
    TextureParameter* sphereTextureParameter4 = new TextureParameter(true, 0, repeat);
    RenderableObject* renderableSphere4 = new RenderableObject(sphere4, sphereTextureParameter4);
    scene3.renderableObjects.push_back(renderableSphere4);

    SphereCollider sphereCollider4(&sphere4_transform, sphere4_transform.scale);
    scene3.spheres.push_back(&sphereCollider4);

    scene3.Start = Start3;
    scene3.Update = UpdateScene3;
    scenes.push_back(&scene3);
    std::cout << "Scene 3: loading complete" << std::endl;

    int sceneIndex = 0;

    activeScene = scenes[sceneIndex];

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
        if(activeScene == &scene3){
            ImGui::SliderFloat("Cutting Distance Multiplier", &cuttingDistanceMultiplier, 1.0f, 10.0f);
        }
        ImGui::NewLine;
        ImGui::SliderInt("Force Shader to use", &shaderNumber, 0, 2);
        if(shaderNumber == 0){
            ImGui::Text("BlinnPhong");
        } else if(shaderNumber == 1) {
            ImGui::Text("GGX");
        } else if(shaderNumber == 2) {
            ImGui::Text("Color");  
        }

        // activeScene
        ImGui::NewLine;
        ImGui::SliderInt("Scene active: ", &sceneIndex, 0, 2);
        if(previousActiveScene != scenes[sceneIndex]){
            ChangeScene(scenes[sceneIndex]);
        }
        ImGui::Text((std::to_string(sceneIndex + 1)).c_str());


        ImGui::NewLine;
        ImGui::Text("Physic Simulation");
        ImGui::NewLine;
        ImGui::SliderFloat("Gravity", &gravity, -0.0f, -9.8f);
        ImGui::NewLine;
        ImGui::SliderFloat("Mass", &mass, 0.0f, 2.0f);

        ImGui::NewLine;
        ImGui::Text("Constraints");
        ImGui::NewLine;
        if(ImGui::SliderInt("type", &type, 0, 2)){
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
                // springType = PHYSICAL_ADVANCED;
                K = 7.5f;
                U = 0.1f;
                gravity = -9.8f;
                constraintIterations = 1;
                constraintIterations = 5;
                constraintLevel = 2;
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
                cloth.PhysicsSteps(activeScene);
                physIter++;
                cloth.CheckForCuts();

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
            color_shader.Use();
            ColorGGXShaderSetup(color_shader, clothTransform, projection, view);
        }

        cloth.Draw();
        
        if(!pKeyPressed && once)
        {
            cloth.~Cloth();
            pinned = !pinned;
            new(&cloth) Cloth(clothDim, particleOffset, startingPosition, &clothTransform, pinned, springType, K, U, constraintIterations, gravity, mass, collisionIterations, constraintLevel, cuttingDistanceMultiplier);
            once = false;
            //DebugLogStatus();
            //cloth.CutAHole(4 + iter, 4 + iter);
            iter++;
        } else if(pKeyPressed && !once){
            once = true; 
        }

        activeScene->Update(activeScene);
        RenderScene(illumination_shader, *activeScene, projection, view);

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

    // pressing a key between 1 and 5, we change the shader applied to the models
    if((key >= GLFW_KEY_1 && key <= GLFW_KEY_5) && action == GLFW_PRESS)
    {
        // "1" to "5" -> ASCII codes from 49 to 57
        // we subtract 48 (= ASCII CODE of "0") to have integers from 1 to 5
        // we subtract 1 to have indices from 0 to 4 in the shaders list
        unsigned int pressedInt = (key-'0'-1);

        // switch (pressedInt)
        // {
        // case 1:
        //     activeScene = &scene1;
        //     break;
        // case 2:
        //     activeScene = &scene2;
        //     break;
        // case 3:
        //     activeScene = &scene3;
        //     break;            
        // default:
        //     activeScene = &scene1;
        //     break;
        // }
    }

    // if R is pressed, we start/stop the animated rotation of models
    if(key == GLFW_KEY_R && action == GLFW_PRESS){
        R_KEY = true;
        pausePhysics = !pausePhysics;
    }

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;

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

void UpdateScene1 (Scene* scene){

    if(keys[GLFW_KEY_UP])
    {
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, glm::vec3(camera.Front.x, 0.0f, camera.Front.z), action);
    }
    if(keys[GLFW_KEY_DOWN])
    {
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, -glm::vec3(camera.Front.x, 0.0f, camera.Front.z), action);
    }
    if(keys[GLFW_KEY_LEFT])
    {
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, -camera.Right, action);
    }
    if(keys[GLFW_KEY_RIGHT])
    {
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, camera.Right, action);
    }
    if(keys[GLFW_KEY_SPACE]){
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, camera.WorldUp, action);
    }
    if(keys[GLFW_KEY_LEFT_CONTROL]){
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, -camera.WorldUp, action);
    }
}
void UpdateScene2 (Scene* scene){
    // Coat attached to the sphere

    // // Moving sphere
    // if(scene->renderableObjects[0]->gameObject->transform->translation.z > 5.0f){
    //     direction = glm::vec3(0.0f, 0.0f, -1.0f);
    // } else if(scene->renderableObjects[0]->gameObject->transform->translation.z <= -5.0f){
    //     direction = glm::vec3(0.0f, 0.0f, 1.0f);
    // }

    // scene->renderableObjects[0]->gameObject->transform->translation += direction * sphereSpeed * deltaTime;
    // // scene->renderableObjects[1]->gameObject->transform->translation -= direction * sphereSpeed;

    // MoveClothPinnedParticles(direction * sphereSpeed * deltaTime);

    if(keys[GLFW_KEY_UP])
    {
        MoveSphereAndCloth(scene->renderableObjects[0]->gameObject->transform, glm::vec3(camera.Front.x, 0.0f, camera.Front.z), action);
        MoveSphere(scene->renderableObjects[1]->gameObject->transform, glm::vec3(camera.Front.x, 0.0f, camera.Front.z), action);
    }
    if(keys[GLFW_KEY_DOWN])
    {
        MoveSphereAndCloth(scene->renderableObjects[0]->gameObject->transform, -glm::vec3(camera.Front.x, 0.0f, camera.Front.z), action);
        MoveSphere(scene->renderableObjects[1]->gameObject->transform, -glm::vec3(camera.Front.x, 0.0f, camera.Front.z), action);
    }
    if(keys[GLFW_KEY_LEFT])
    {
        MoveSphereAndCloth(scene->renderableObjects[0]->gameObject->transform, -camera.Right, action);
        MoveSphere(scene->renderableObjects[1]->gameObject->transform, -camera.Right, action);
    }
    if(keys[GLFW_KEY_RIGHT])
    {
        MoveSphereAndCloth(scene->renderableObjects[0]->gameObject->transform, camera.Right, action);
        MoveSphere(scene->renderableObjects[1]->gameObject->transform, camera.Right, action);
    }
    if(keys[GLFW_KEY_SPACE]){
        MoveSphereAndCloth(scene->renderableObjects[0]->gameObject->transform, camera.WorldUp, action);
        MoveSphere(scene->renderableObjects[1]->gameObject->transform, camera.WorldUp, action);
    }
    if(keys[GLFW_KEY_LEFT_CONTROL]){
        MoveSphereAndCloth(scene->renderableObjects[0]->gameObject->transform, -camera.WorldUp, action);
        MoveSphere(scene->renderableObjects[1]->gameObject->transform, -camera.WorldUp, action);
    }


}
void UpdateScene3 (Scene* scene){
    if(keys[GLFW_KEY_UP])
    {
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, glm::vec3(camera.Front.x, 0.0f, camera.Front.z), action);
    }
    if(keys[GLFW_KEY_DOWN])
    {
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, -glm::vec3(camera.Front.x, 0.0f, camera.Front.z), action);
    }
    if(keys[GLFW_KEY_LEFT])
    {
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, -camera.Right, action);
    }
    if(keys[GLFW_KEY_RIGHT])
    {
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, camera.Right, action);
    }
    if(keys[GLFW_KEY_SPACE]){
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, camera.WorldUp, action);
    }
    if(keys[GLFW_KEY_LEFT_CONTROL]){
        MoveSphere(scene->renderableObjects[0]->gameObject->transform, -camera.WorldUp, action);
    }

    // if(keys[GLFW_KEY_E]){
    //     c->CutAHole(c->dim - 5, c->dim - 5);
    // }
}
void ChangeScene(Scene* sceneToChange){
    previousActiveScene = activeScene;
    activeScene = sceneToChange;
    activeScene->Start(activeScene);
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
void MoveSphere(Transform* sphere_transform , glm::vec3 direction, int action){
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

    sphere_transform->translation += direction;

    direction = glm::vec3(0.0f);
}
void MoveSphereAndCloth(Transform* sphere_transform , glm::vec3 direction, int action){
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

    sphere_transform->translation += direction;

    MoveClothPinnedParticles(direction);

    // direction = glm::vec3(0.0f);
}
void MoveClothPinnedParticles(glm::vec3 direction){
    // Move pinned particles
    for(int i = 0; i < clothDim; i++){
        for(int j = 0; j < clothDim; j++){
            Particle* p = (*c).getParticle(i, j, clothDim);
            if(p->movable == false){
                p->pos += direction;
            }
        }
    }
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
void ColorGGXShaderSetup(Shader colorGGXShader, Transform clothTransform, glm::mat4 projection, glm::mat4 view){

    SetUpClothShader(colorGGXShader, clothTransform, projection, view);

    GLint lightDirLocation = glGetUniformLocation(colorGGXShader.Program, "lightVector");
    GLint kdLocation = glGetUniformLocation(colorGGXShader.Program, "Kd");
    GLint alphaLocation = glGetUniformLocation(colorGGXShader.Program, "alpha");
    GLint f0Location = glGetUniformLocation(colorGGXShader.Program, "F0");
    
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

void Start1(Scene* scene){
    glClearColor(0.26f, 0.46f, 0.98f, 1.0f);
    
    c->getParticle(c->dim-1, 0, c->dim)->movable = true;
    c->getParticle(c->dim-1, 1, c->dim)->movable = true;

    c->getParticle(c->dim-1, c->dim-1, c->dim)->movable = true;
    c->getParticle(c->dim-1, c->dim-2, c->dim)->movable = true;


    int numOfConstraints = c->constraints.capacity();
    for(int i = 0; i < numOfConstraints; i++){
        c->constraints[i].cuttable = false;
    }

}
void Start2(Scene* scene){
    glClearColor(0.702f, 0.929f, 0.655f, 1.0f);
    
    c->getParticle(c->dim-1, 0, c->dim)->movable = true;
    c->getParticle(c->dim-1, 1, c->dim)->movable = true;

    c->getParticle(c->dim-1, c->dim-1, c->dim)->movable = true;
    c->getParticle(c->dim-1, c->dim-2, c->dim)->movable = true;


    int numOfConstraints = c->constraints.capacity();
    for(int i = 0; i < numOfConstraints; i++){
        c->constraints[i].cuttable = false;
    }

}
void Start3(Scene* scene){
    glClearColor(0.953f, 0.8f, 1.0f, 1.0f);


    // Init cloth fixed in 4 points
    
    c->getParticle(c->dim-1, 0, c->dim)->movable = false;
    c->getParticle(c->dim-1, 1, c->dim)->movable = false;

    c->getParticle(c->dim-1, c->dim-1, c->dim)->movable = false;
    c->getParticle(c->dim-1, c->dim-2, c->dim)->movable = false;

    int numOfConstraints = c->constraints.capacity();
    for(int i = 0; i < numOfConstraints; i++){
        c->constraints[i].cuttable = true;
    }
}