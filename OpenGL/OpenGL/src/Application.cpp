#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <fstream>
#include <string>
#include <sstream>
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "VertexBufferLayout.h"
// https://docs.gl/

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    // Create from the core (whatever that means)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // opengl version 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "First Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Sync with monitor refresh rate */ 
    glfwSwapInterval(1);

    // Make sure glew is ok (look at glew documentation)
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW Error!" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    // The origin point of the refernce system of the position
    // is in bot-left corner of the window

    // This scope will help us avoid OpenGL GetError, getting the error
    //  because we did not de-allocate from the stack the index and vertex buffer
    //  but we cannot do it cause glfwTerminate clean up the glfw scope
    //  this cause an infinite loop
    {   
        // Vertex are not exclusively boundend to a positin
        // They may have multiple attributes
        // One of the can be the position
        // 2 index for each vertex
        float positions[] = {
            -0.5f, -0.5f, // 0
             0.5f, -0.5f, // 1
             0.5f,  0.5f, // 2
            -0.5f,  0.5f, // 3
        };

        // Not to keep multiple instances of the same vertex, we can just keep and index of them and reuse them
        unsigned int indicies[] = {
            0, 1, 2, // first triangle
            2, 3, 0  // second triangle
        };

        // Creating a vertex object array
        unsigned int vao;
        GlCall(glGenVertexArrays(1, &vao));
        GlCall(glBindVertexArray(vao));

        VertexArray va;
        // VERTEX BUFFER
        VertexBuffer vb(positions, 4 * 2 * sizeof(float));
        // We dont really need to bind it, since we already do it in the constructor

        VertexBufferLayout layout;
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

        // INDEX BUFFER
        // we want to send it to the GPU
        IndexBuffer ib(indicies, 6);

        Shader shader("res/shaders/Basic.shader");
        shader.Bind();
        shader.SetUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f); // retrive the unform named u_Color and sending 4 floats

        // unbound everything
        // like we have multiple objects to render
        va.Unbind();
        shader.Unbind();
        vb.Unbind();
        ib.Unbind();

        Renderer renderer;


        float r = 0.0f; // RED channel
        float increment = 0.05f;
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            renderer.Clear();

            shader.Bind();
            // Maybe better to use a Material
            shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);

            va.Bind();
            ib.Bind();

            // Draw without an index array (Number of vertexes
            // glDrawArrays(GL_TRIANGLES, 0, 6);
            renderer.Draw(va, ib, shader);

            if (r > 1.0f) {
                increment = -0.05f;
            }
            else if (r < 0.0f) {
                increment = 0.05f;
            }

            r += increment;

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}