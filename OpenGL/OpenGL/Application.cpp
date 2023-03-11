#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <fstream>
#include <string>
#include <sstream>
// https://docs.gl/

// MACRO
// ASSERT, when found insert a break point
// __debugbreak works only on MVSC not wiht gcc or other compilers
#define ASSERT(x) if (!(x)) __debugbreak();
// GlCall will be attached to every gl funcitons that we are gona use to chek for errors
// __FILE__ and __LINE__ are intrinsic to all compilers
// It will not work for single line if
#define GlCall(x) GlClearError();\
    x;\
    ASSERT(GlLogCall(#x, __FILE__, __LINE__))

static void GlClearError() {
    while(glGetError() != GL_NO_ERROR) {
    }
}

static bool GlLogCall(const char* function, const char* file, int line) {
    // As long as error is not ZERO (GL_NO_ERROR)
    while (GLenum error = glGetError()) { 
        std::cout << "[OpenGL Error] ( " << error << " )" << function << 
            " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

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

    GlCall(
        /* Initialize the library */
        if (!glfwInit())
            return -1;
    );

    GlCall(
        /* Create a windowed mode window and its OpenGL context */
        window = glfwCreateWindow(640, 480, "First Triangle", NULL, NULL);
        if (!window)
        {
            glfwTerminate();
            return -1;
        }
    );
    

    /* Make the window's context current */
    GlCall(glfwMakeContextCurrent(window));

    GlCall(
        // Make sure glew is ok (look at glew documentation)
        if (glewInit() != GLEW_OK) {
            std::cout << "GLEW Error!" << std::endl;
        }
    );

    std::cout << glGetString(GL_VERSION) << std::endl;

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

    // The origin point of the refernce system of the position
    // is in bot-left corner of the window

    // VERTEX BUFFER
    // Creating a buffer
    unsigned int buffer;
    // Generate 1 vertex buffer
    GlCall(glGenBuffers(1, &buffer));
    // Selcet buffer by BINDING
    GlCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    // Specify the data
    GlCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

    // 0 => First index
    // 2 => number of index of each vertex position
    // type
    // do we need to normalize those? FALSE
    // size to get tot the next vertex
    // the offset
    GlCall(glEnableVertexAttribArray(0));
    GlCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (const void*)0));

    // INDEX BUFFER
    // we want to send it to the GPU
    unsigned int ibo; // index buffer object
    GlCall(glGenBuffers(1, &ibo));
    GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GlCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indicies, GL_STATIC_DRAW));

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    std::cout << "VertexSource" << std::endl;
    std::cout << source.VertexSource << std::endl;
    std::cout << "FragmentSource" << std::endl;
    std::cout << source.FragmentSource << std::endl;


    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    GlCall(glUseProgram(shader));

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GlCall(glClear(GL_COLOR_BUFFER_BIT));

        // Clear errors before calling the specific function
        GlClearError();

        // Draw without an index array (Number of vertexes
        // glDrawArrays(GL_TRIANGLES, 0, 6);
        GlCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr)); // used if we have and index buffer

        /* Swap front and back buffers */
        GlCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GlCall(glfwPollEvents());
    }

    GlCall(glDeleteProgram(shader));

    GlCall(glfwTerminate());
    return 0;
}