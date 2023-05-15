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

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// dimensions of application's window
GLuint screenWidth = 1200, screenHeight = 900;

// callback functions for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// setup of Shader Programs for the 5 shaders used in the application
void SetupShaders();
// delete Shader Programs whan application ends
void DeleteShaders();
// print on console the name of current shader
void PrintCurrentShader(int shader);

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// rotation angle on Y axis
GLfloat orientationY = 0.0f;
// rotation speed on Y axis
GLfloat spin_speed = 30.0f;
// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

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


#include <utils/Cloth.h>

// Just below are three global variables holding the actual animated stuff; Cloth and Ball 
Cloth cloth1(14,10,55,45); // one Cloth object of the Cloth class
Vec3 ball_pos(7,-5,0); // the center of our one ball
float ball_radius = 2; // the radius of our one ball



/***** Below are functions Init(), display(), reshape(), keyboard(), arrow_keys(), main() *****/

/* This is where all the standard Glut/OpenGL stuff is, and where the methods of Cloth are called; 
addForce(), windForce(), timeStep(), ballCollision(), and drawShaded()*/


void init(GLvoid)
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.2f, 0.2f, 0.4f, 0.5f);				
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_COLOR_MATERIAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat lightPos[4] = {-1.0,1.0,0.5,0.0};
	glLightfv(GL_LIGHT0,GL_POSITION,(GLfloat *) &lightPos);

	glEnable(GL_LIGHT1);

	GLfloat lightAmbient1[4] = {0.0,0.0,0.0,0.0};
	GLfloat lightPos1[4] = {1.0,0.0,-0.2,0.0};
	GLfloat lightDiffuse1[4] = {0.5,0.5,0.3,0.0};

	glLightfv(GL_LIGHT1,GL_POSITION,(GLfloat *) &lightPos1);
	glLightfv(GL_LIGHT1,GL_AMBIENT,(GLfloat *) &lightAmbient1);
	glLightfv(GL_LIGHT1,GL_DIFFUSE,(GLfloat *) &lightDiffuse1);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
}


float ball_time = 0; // counter for used to calculate the z position of the ball below

/* display method called each frame*/
void display(void)
{
	// calculating positions

	ball_time++;
	ball_pos.f[2] = cos(ball_time/50.0)*7;

	cloth1.addForce(Vec3(0,-0.2,0)*TIME_STEPSIZE2); // add gravity each frame, pointing down
	cloth1.windForce(Vec3(0.5,0,0.2)*TIME_STEPSIZE2); // generate some wind each frame
	cloth1.timeStep(); // calculate the particle positions of the next frame
	cloth1.ballCollision(ball_pos,ball_radius); // resolve collision with the ball



	// drawing

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glDisable(GL_LIGHTING); // drawing some smooth shaded background - because I like it ;)
	glBegin(GL_POLYGON);
	glColor3f(0.8f,0.8f,1.0f);
	glVertex3f(-200.0f,-100.0f,-100.0f);
	glVertex3f(200.0f,-100.0f,-100.0f);
	glColor3f(0.4f,0.4f,0.8f);	
	glVertex3f(200.0f,100.0f,-100.0f);
	glVertex3f(-200.0f,100.0f,-100.0f);
	glEnd();
	glEnable(GL_LIGHTING);

	glTranslatef(-6.5,6,-9.0f); // move camera out and center on the cloth
	glRotatef(25,0,1,0); // rotate a bit to see the cloth from the side
	cloth1.drawShaded(); // finally draw the cloth with smooth shading
	
	// glPushMatrix(); // to draw the ball we use glutSolidSphere, and need to draw the sphere at the position of the ball
	// glTranslatef(ball_pos.f[0],ball_pos.f[1],ball_pos.f[2]); // hence the translation of the sphere onto the ball position
	// glColor3f(0.4f,0.8f,0.5f);
	// glutSolidSphere(ball_radius-0.1,50,50); // draw the ball, but with a slightly lower radius, otherwise we could get ugly visual artifacts of cloth penetrating the ball slightly
	// glPopMatrix();

	// glutSwapBuffers();
	// glutPostRedisplay();
}

void reshape(int w, int h)  
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();  
	// if (h==0)  
	// 	gluPerspective(80,(float)w,1.0,5000.0);
	// else
	// 	gluPerspective (80,( float )w /( float )h,1.0,5000.0 );
	glMatrixMode(GL_MODELVIEW);  
	glLoadIdentity(); 
}



/////////////////// MAIN function ///////////////////////
int main()
{
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
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "RGP_lecture03a", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // we put in relation the window and the callbacks
    glfwSetKeyCallback(window, key_callback);

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
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

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

    init();

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

        display();
        reshape(screenWidth, screenHeight);

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
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
        spinning=!spinning;

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
}