#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <fstream>
#include <string>
#include <sstream>
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
// https://docs.gl/

struct ShaderProgramSource {
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath) {
    // open it in modern c++
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) 
    {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") !=  std::string::npos) {
                // set mode to vertex
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") !=  std::string::npos) {
                // set mode to fragment
                type = ShaderType::FRAGMENT;
            }
            
        }
        else
        {
            ss[(int)type] << line << "\n";
        }
    }

    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
    GlCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    GlCall(glShaderSource(id, 1, &src, nullptr));
    GlCall(glCompileShader(id));

    // Error Handling 
    // Query compiled shader
    int result;
    GlCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE) 
    {
        int length;
        GlCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        // alloca: alloc on the stack dynamically
        char* message = (char*)alloca(length* sizeof(char));
        GlCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Error compiling shader: " <<
            (type == GL_VERTEX_SHADER ? "vetex" : "fragment")
            << std::endl;
        std::cout << message << std::endl;
        GlCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, 
    const std::string& fragmentShader) {
    // Provide sourcecode of shader in a string
    // Compile the 2 togheter and give a unique ID

    GlCall(unsigned int program = glCreateProgram());
    GlCall(unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader));
    GlCall(unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader));
    
    GlCall(glAttachShader(program, vs));
    GlCall(glAttachShader(program, fs));
    GlCall(glLinkProgram(program));
    GlCall(glValidateProgram(program));

    GlCall(glDeleteShader(vs));
    GlCall(glDeleteShader(fs));

    return program;
}

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

        // The origin point of the refernce system of the position
        // is in bot-left corner of the window

        // VERTEX BUFFER
        VertexBuffer vb(positions, 4 * 2 * sizeof(float));
        // We dont really need to bind it, since we already do it in the constructor

        // BUFFER LAYOUT
        GlCall(glEnableVertexAttribArray(0));
        GlCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (const void*)0)); // Link the buffer (the last one) to the vao in vao[0]

        // INDEX BUFFER
        // we want to send it to the GPU
        IndexBuffer ib(indicies, 6);

        ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
        std::cout << "VertexSource" << std::endl;
        std::cout << source.VertexSource << std::endl;
        std::cout << "FragmentSource" << std::endl;
        std::cout << source.FragmentSource << std::endl;
        unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
        GlCall(glUseProgram(shader));

        // Get the location of the uniform
        int location = glGetUniformLocation(shader, "u_Color"); // retrive the unform named u_Color
        ASSERT(location != -1); // if -1 not found uniform
        GlCall(glUniform4f(location, 0.8f, 0.3f, 0.8f, 1.0f));  // sending 4 floats

        // unbound everything
        // like we have multiple objects to render
        GlCall(glBindVertexArray(0));
        GlCall(glUseProgram(0));
        GlCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        float r = 0.0f; // RED channel
        float increment = 0.05f;
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            GlCall(glClear(GL_COLOR_BUFFER_BIT));

            GlCall(glUseProgram(shader));   // Bind shader
            GlCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));  // sending 4 float

            GlCall(glBindVertexArray(vao));

            ib.Bind();

            // Draw without an index array (Number of vertexes
            // glDrawArrays(GL_TRIANGLES, 0, 6);
            GlCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr)); // used if we have and index buffer

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

        glDeleteProgram(shader);
    }
    glfwTerminate();
    return 0;
}