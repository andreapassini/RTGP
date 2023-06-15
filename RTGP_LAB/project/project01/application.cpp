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


GLFWwindow* window;
GLuint screenWidth = 1200, screenHeight = 900;

int SetupOpenGL();
void SetupShaders();
void DebugLogStatus();

void RenderScene1(Shader &shader, glm::mat4 projection, glm::mat4 view, Transform &planeTransform, Model &planeModel, Transform &sphereTransform, Model &sphereModel);

GLint LoadTexture(const char* path);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void apply_camera_movements();
void imGuiSetup(GLFWwindow *window);
void PrintVec3(glm::vec3* vec);

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

glm::vec3 startingPosition(0.0f, 0.0f, 0.0f);

bool once = true;
unsigned int iter = 0;

int clothDim = 30;
float particleOffset = 0.15f;
bool clothExist = true;
unsigned int prints = 0;
bool pinned = true;
ConstraintType springType = POSITIONAL;
float gravity = -9.8f;
float mass = 1.0f;
float K = 0.5f;
float U = 0.1f;
int constraintIterations = 15;
int collisionIterations = 15;

unsigned int windowSize = 10;
unsigned int overlap = 3;

glm::vec3 spherePosition(3.0f, -4.5f, -2.5f);
glm::vec3 cubePosition(3.0f, -4.5f, -2.5f);
glm::vec3 planePosition(0.0f, -10.0f, 0.0f);

PerformanceCalculator performanceCalculator(windowSize, overlap);

int main()
{
    if(SetupOpenGL() == -1)
        return -1;

    Shader illumination_shader = Shader("06_illumination.vert", "06_illumination.frag");
    Shader force_shader = Shader("07_force.vert", "07_force.frag");
    Shader fullColor_shader = Shader("00_basic.vert", "01_fullcolor.frag");

    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    // View matrix (=camera): position, view direction, camera "up" vector
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    Model sphereModel("../../models/sphere.obj");
    Model planeModel("../../models/plane.obj");
    Model cubeModel("../../models/cube.obj");

    textureID.push_back(LoadTexture("../../textures/UV_Grid_Sm.png"));
    textureID.push_back(LoadTexture("../../textures/SoilCracked.png"));

    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    Transform sphereTransform(view);
    SphereCollider sphereCollider(&sphereTransform, 1.0f);
    std::vector<SphereCollider> sphereColliders;
    sphereColliders.push_back(sphereCollider);
    std::cout << "Line: " << __LINE__ << std::endl;
    positionZ = 0.8f;
    GLint directionZ = 1;

    Transform planeTransform(view);

    Transform cubeTransform(view);

    Transform clothTransform(view);
    Cloth cloth(clothDim, particleOffset, startingPosition, &clothTransform, pinned, springType, K, U, constraintIterations, gravity, mass, collisionIterations);

    PerformanceCalculator performanceCalculator(windowSize, overlap);

    int type = 0;   // Used in ImGui display
    
    PhysicsSimulation physicsSimulation;
    float currentTime = glfwGetTime();
    physicsSimulation.StartPhysicsSimulation(currentTime);

    //physicsSimulation.AddObjectToPhysicWorld(&sphereTransform, 1.0f, false);

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
                constraintIterations = 15;
                break;
            case 1:
                // springType = PHYSICAL;
                K = 2.5f;
                gravity = -0.8f;
                constraintIterations = 1;
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
                gravity = -0.8f;
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
        ImGui::SliderFloat("K", &K, 0.01f, 5.0f);

        ImGui::NewLine;
        ImGui::SliderInt("Constraint Iterations", &constraintIterations, 0, 25);

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
        force_shader.Use();

        glUniformMatrix4fv(glGetUniformLocation(force_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(force_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        //cloth.windForce(glm::vec3(0.0f, 0.0f, 1.0f)*3.5f);
        clothTransform.Transformation(
            glm::vec3(1.0f, 1.0f, 1.0f),
            0.0f, glm::vec3(0.0f, 1.0f, 0.0f),
            startingPosition,
            view
        );

        unsigned int maxIter = 4U;
        unsigned int physIter = 0U;

        while(!physicsSimulation.isPaused &&  currentTime > physicsSimulation.getVirtualTIme()){
            cloth.ResetShaderForce();
            physicsSimulation.AddForceToAll(glm::vec3(0.0f, gravity, 0.0f));
            cloth.AddGravityForce();
            physicsSimulation.FixedTimeStep(currentTime);
            //cloth.PhysicsSteps(sphereCollider, planePosition.y + 0.1f);
            //cloth.PhysicsSteps(deltaTime, (glm::vec4(spherePosition, 1.0f) * sphereTransform.modelMatrix), 1.0f, planePosition.y + 0.1f);
            cloth.PhysicsSteps((glm::vec4(spherePosition, 1.0f) * sphereTransform.modelMatrix), 1.0f, planePosition.y + 0.1f);
            physIter++;

            if(physIter > maxIter){
                std::cout << "Physics Simulation lagging " << std::endl;
                physicsSimulation.SynchVirtualTime(currentTime);
                break;
            }
        }
        
        //cloth.PhysicsSteps(deltaTime, (glm::vec4(spherePosition, 1.0f) * sphereTransform.modelMatrix), 1.0f, planePosition.y + 0.1f);
        glUniformMatrix4fv(glGetUniformLocation(force_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(clothTransform.modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(force_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(clothTransform.normalMatrix));
        cloth.Draw();
        
        if(!pKeyPressed && once)
        {
            cloth.~Cloth();
            pinned = !pinned;
            new(&cloth) Cloth(clothDim, particleOffset, startingPosition, &clothTransform, pinned, springType, K, U, constraintIterations, gravity, mass, collisionIterations);
            once = false;
            //DebugLogStatus();
            //cloth.CutAHole(4 + iter, 4 + iter);
            iter++;
        } else if(pKeyPressed && !once){
            once = true; 
        }


        // OBJECTS
        RenderScene1(illumination_shader, projection, view, planeTransform, planeModel, sphereTransform, sphereModel);

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
    }

    // if R is pressed, we start/stop the animated rotation of models
    if(key == GLFW_KEY_R && action == GLFW_PRESS){
        R_KEY = true;
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
void RenderScene1(Shader &shader, glm::mat4 projection, glm::mat4 view, Transform &planeTransform, Model &planeModel, Transform &sphereTransform, Model &sphereModel){

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

    sphereTransform.Transformation(
        glm::vec3(1.0f, 1.0f, 1.0f),
        0.0f, glm::vec3(0.0f, 1.0f, 0.0f),
        spherePosition,
        view);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereTransform.modelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereTransform.normalMatrix));
    sphereModel.Draw();

    
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