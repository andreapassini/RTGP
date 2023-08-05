#pragma once

#include "../glm/glm.hpp"
#include "../glad/glad.h"

class TextureParameter
{
private:
public:
    bool useTexture;
    GLint textureId;
    GLfloat repeat;

    TextureParameter(bool use_texture, GLint textId, GLfloat repeatParameter){
        this->useTexture = use_texture;
        this->textureId = textId;
        this->repeat = repeatParameter;
    };
    ~TextureParameter();
};