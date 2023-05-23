/*
Es03a: Basic shaders, animation of rotation transform, wireframe visualization
- swapping between different basic shaders pressing keys from 1 to 5

N.B. 1)
The lecture uses a very "basic" shader swapping method.
This method was used until OpenGL 3.3 (thus, still valid for OpenGL ES and WebGL), and it can still be used for simple scenes and few shaders.
In the next lectures, a more advances method based on Shader Subroutines will be used.

N.B. 2) no texturing in this version of the classes

N.B. 3) to test different parameters of the shaders, it is convenient to use some GUI library, like e.g. Dear ImGui (https://github.com/ocornut/imgui)

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2022/2023
Master degree in Computer Science
Universita' degli Studi di Milano
*/

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

// Std. Includes
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

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

// classes developed during lab lectures to manage shaders and to load models
#include <utils/shader.h>
#include <utils/model.h>
#include <utils/camera.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// My classes
#include <utils/Transform.h>
#include <utils/cloth.h>

// dimensions of application's window
GLuint screenWidth = 1200, screenHeight = 900;

// callback functions for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements();

// setup of Shader Programs for the 5 shaders used in the application
void SetupShaders();
// delete Shader Programs whan application ends
void DeleteShaders();
// print on console the name of current shader
void PrintCurrentShader(int shader);

int SetupOpenGL();

// we initialize an array of booleans for each keyboard key
bool keys[1024];

// we need to store the previous mouse position to calculate the offset with the current frame
GLfloat lastX, lastY;
// when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
bool firstMouse = true;

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// rotation angle on Y axis
GLfloat orientationY = 0.0f;
// rotation speed on Y axis
GLfloat spin_speed = 30.0f;
// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;

GLfloat positionZ = 0.0f;
GLfloat movement_speed = 5.0f;
GLboolean movingOnX = GL_TRUE;
GLboolean instantiate = GL_FALSE;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// we create a camera. We pass the initial position as a parameter to the constructor. The last boolean tells if we want a camera "anchored" to the ground
Camera camera(glm::vec3(0.0f, 0.0f, 7.0f), GL_FALSE);

// enum data structure to manage indices for shaders swapping
enum available_ShaderPrograms{ FULLCOLOR, FLATTEN, NORMAL2COLOR, WAVE, UV2COLOR };
// strings with shaders names to print the name of the current one on console
const char * print_available_ShaderPrograms[] = { "FULLCOLOR", "FLATTEN", "NORMAL2COLOR", "WAVE", "UV2COLOR" };

// index of the current shader (= 0 in the beginning)
GLuint current_program = FULLCOLOR;
// a vector for all the Shader Programs used and swapped in the application
vector<Shader> shaders;

// Uniforms to pass to shaders
// color to be passed to Fullcolor and Flatten shaders
GLfloat myColor[] = {1.0f,0.0f,0.0f};
// weight and velocity for the animation of Wave shader
GLfloat weight = 0.2f;
GLfloat speed = 5.0f;


// OpenGL Setup
GLFWwindow* window;

glm::vec3 startingPosition(0.0f, 3.0f, -3.0f);

/////////////////// MAIN function ///////////////////////
int main()
{
    if(SetupOpenGL() == -1)
        return -1;

    // we create the Shader Programs used in the application
    SetupShaders();
    // we print on console the name of the first shader used
    PrintCurrentShader(current_program);

    // we set projection and view matrices
    // N.B.) in this case, the camera is fixed -> we set it up outside the rendering loop
    // Projection matrix: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    // View matrix (=camera): position, view direction, camera "up" vector
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // we load the model(s) (code of Model class is in include/utils/model.h)
    Model sphereModel("../../models/sphere.obj");
    Model planeModel("../../models/plane.obj");
    Model cubeModel("../../models/cube.obj");

    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    Transform sphereTransform(view);
    positionZ = 0.8f;
    GLint directionZ = 1;

    Transform planeTransform(view);
    Transform cubeTransform(view);

    bool once = true;
    unsigned int prints = 0;
    Cloth cloth(8.0f, 0.25f, startingPosition);
    cloth.PrintParticles(prints);
    Transform clothTransform(view);

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

        // Check is an I/O event is happening
        glfwPollEvents();
        // we apply FPS camera movements
        apply_camera_movements();
        // View matrix (=camera): position, view direction, camera "up" vector
        view = camera.GetViewMatrix();

        // we "clear" the frame and z buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // we set the rendering mode
        if (wireframe)
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // if animated rotation is activated, then we increment the rotation angle using delta time and the rotation speed parameter
        if (spinning)
            orientationY+=(deltaTime*spin_speed);

        // Moving Sphere on X axis
        if(movingOnX){
            if(positionZ >= 6 || positionZ <= -6)
                directionZ *= -1;

            positionZ += directionZ * (deltaTime * movement_speed);
        }

        // We "install" the selected Shader Program as part of the current rendering process
        shaders[current_program].Use();

        // uniforms are passed to the corresponding shader
        if (current_program == FULLCOLOR || current_program == FLATTEN)
        {
            // we determine the position in the Shader Program of the uniform variable
            GLint fragColorLocation = glGetUniformLocation(shaders[current_program].Program, "colorIn");
            // we assign the value to the uniform variable
            glUniform3fv(fragColorLocation, 1, myColor);
        }
        else if (current_program == WAVE)
        {
            // we determine the position in the Shader Program of the uniform variables
            GLint weightLocation = glGetUniformLocation(shaders[current_program].Program, "weight");
            GLint timerLocation = glGetUniformLocation(shaders[current_program].Program, "timer");
            // we assign the value to the uniform variables
            glUniform1f(weightLocation, weight);
            glUniform1f(timerLocation, currentFrame*speed);
        }

        // we pass projection and view matrices to the Shader Program
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        //SPHERE
        /*
          we create the transformation matrix
          N.B.) the last defined is the first applied

          We need also the matrix for normals transformation, which is the inverse of the transpose of the 3x3 submatrix (upper left) of the modelview.
          We do not consider the 4th column because we do not need translations for normals.
          An explanation (where XT means the transpose of X, etc):
            "Two column vectors X and Y are perpendicular if and only if XT.Y=0.
            If we're going to transform X by a matrix M, we need to transform Y by some matrix N so that (M.X)T.(N.Y)=0.
            Using the identity (A.B)T=BT.AT, this becomes (XT.MT).(N.Y)=0 => XT.(MT.N).Y=0.
            If MT.N is the identity matrix then this reduces to XT.Y=0.
            And MT.N is the identity matrix if and only if N=(MT)-1, i.e. N is the inverse of the transpose of M.

        */

       //SPHERE
        sphereTransform.Transformation(
            glm::vec3(0.8f, 0.8f, 0.8f),
            orientationY, glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3( 0.0f, 0.0f, positionZ),
            view);
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereTransform.modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereTransform.normalMatrix));
        sphereModel.Draw();
        

        // PLANE
        planeTransform.Transformation(
            glm::vec3(10.0f, 1.0f, 10.0f),
            0.0f, glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, -2.0f, 0.0f),
            view
        );
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeTransform.modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeTransform.normalMatrix));
        planeModel.Draw();

        //CUBE
        cubeTransform.Transformation(
            glm::vec3(0.25f, 0.25f, 0.25f),
            orientationY, glm::vec3(0.0f, 1.0f, 0.0f),
            startingPosition,
            view
        );
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(cubeTransform.modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(cubeTransform.normalMatrix));
        cubeModel.Draw();
        

        cloth.AddGravityForce();
        cloth.AddRandomIntensityForce(glm::vec3(1.0f, 0.0f, 0.0f), -0.5f, 5.5f);
        //cloth.windForce(glm::vec3(0.3f, 0.0f, 0.0f));
        auto current_time = Time::now();
        fsec deltaTime = (current_time - start_time);
        start_time = Time::now();

        cloth.PhysicsSteps(deltaTime.count());
        //CLOTH
        clothTransform.Transformation(
            glm::vec3(1.0f, 1.0f, 1.0f),
            0.0f, glm::vec3(0.0f, 1.0f, 0.0f),
            startingPosition,
            view
        );
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(clothTransform.modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(clothTransform.normalMatrix));
        cloth.Draw();

        if(!spinning && once)
        {
            prints++;
            cloth.PrintParticles(prints);
            once = false;
            std::cout << "deltaTime in sec" << deltaTime.count() << std::endl;
        } else if(spinning && !once){
           once = true; 
        }

        // Swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    DeleteShaders();
    // we close and delete the created context
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

//////////////////////////////////////////
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
    Shader shader5("05_uv2color.vert", "05_uv2color.frag");
    shaders.push_back(shader5);
}

//////////////////////////////////////////
// we delete all the Shaders Programs
void DeleteShaders()
{
    for(GLuint i = 0; i < shaders.size(); i++)
        shaders[i].Delete();
}

//////////////////////////////////////////
// we print on console the name of the currently used shader
void PrintCurrentShader(int shader)
{
    std::cout << "Current shader:" << print_available_ShaderPrograms[shader]  << std::endl;

}

//////////////////////////////////////////
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

//////////////////////////////////////////
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

//////////////////////////////////////////
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
