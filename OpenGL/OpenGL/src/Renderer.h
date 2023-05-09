#pragma once

#include <GL/glew.h>
#include "IndexBuffer.h"
#include "Shader.h"
#include "VertexArray.h"

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

void GlClearError();
bool GlLogCall(const char* function, const char* file, int line);

class Renderer {
public:
    void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
    void Clear() const;
};