#pragma once

#include "utils/gameObject.h"
#include "utils/TextureParameter.h"

class RenderableObject
{
private:
public:
    GameObject* gameObject;
    TextureParameter* textureParameter;

    RenderableObject(GameObject* go, TextureParameter* textParam){
        this->gameObject = go;
        this->textureParameter = textParam;
    };
    ~RenderableObject();
};
