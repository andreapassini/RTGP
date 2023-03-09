#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <chrono>
using namespace std::chrono;


// https://docs.gl/

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "First Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Make sure glew is ok (look at glew documentation)
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW Error!" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    // Vertex are not exclusively boundend to a positin
    // They may have multiple attributes
    // One of the can be the position

    // 2 index for each vertex
    float positions[6] = {
        -0.5f, -0.5f,
         0.0f,  0.5f,
         0.5f, -0.5f,
    };

    // The origin point of the refernce system of the position
    // is in bot-left corner of the window

    unsigned int buffer;
    // Generate 1 vertex buffer
    glGenBuffers(1, &buffer);
    // Selcet buffer by BINDING
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Specify the data
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

    // 0 => First index
    // 2 => number of index of each vertex position
    // type
    // do we need to normalize those? FALSE
    // size to get tot the next vertex
    // the offset
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (const void*)0);
    // Enable the Vertex
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // Time evaluation
        auto start = high_resolution_clock::now();

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw without an index array (Number of vertexes
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // glDrawElements() // used if we have and index buffer

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}