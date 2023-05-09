#include "Renderer.h"
#include <iostream>

void GlClearError() {
    while (glGetError() != GL_NO_ERROR) {
    }
}

bool GlLogCall(const char* function, const char* file, int line) {
    // As long as error is not ZERO (GL_NO_ERROR)
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] ( " << error << " )" << function <<
            " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
{
    shader.Bind();
    va.Bind();
    ib.Bind();

    GlCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr)); // used if we have and index buffer
}

void Renderer::Clear() const
{
    GlCall(glClear(GL_COLOR_BUFFER_BIT));
}
