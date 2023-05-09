#include "Shader.h"
#include "Renderer.h"

#include <iostream>

#include <fstream>
#include <string>
#include <sstream>

Shader::Shader(const std::string& filepath)
	: m_FilePath(filepath), m_RendererID(0)
{
    ShaderProgramSource source = ParseShader(filepath);
    std::cout << "VertexSource" << std::endl;
    std::cout << source.VertexSource << std::endl;
    std::cout << "FragmentSource" << std::endl;
    std::cout << source.FragmentSource << std::endl;
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
     
	//CompileShader();
}

Shader::~Shader()
{
    GlCall(glDeleteProgram(m_RendererID));
}

ShaderProgramSource Shader::ParseShader(const std::string& filepath) {
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
            if (line.find("vertex") != std::string::npos) {
                // set mode to vertex
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
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

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
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


unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {
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
        char* message = (char*)alloca(length * sizeof(char));
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

void Shader::Bind() const
{
    GlCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GlCall(glUseProgram(0));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    GlCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

unsigned int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    GlCall(unsigned int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if (location == -1) {
        std::cout << "Warning: uniform '" << name << " does not exist!" << std::endl;
    }
    
    m_UniformLocationCache[name] = location;

    return location;
}
