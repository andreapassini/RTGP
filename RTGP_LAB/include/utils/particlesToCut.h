#pragma once

#include <vector>
#include <utils/Particle.h>

class ParticlesToCut
{
protected:
    ParticlesToCut(){

    };

    // ~ParticlesToCut();

    static ParticlesToCut* instance;
public:
    std::vector<Particle*> particles;
    ParticlesToCut(ParticlesToCut &other) = delete;

    void operator = (const ParticlesToCut &) = delete;

    static ParticlesToCut *GetInstance();

    void CleanUp(){
        particles.clear();
    }
};

ParticlesToCut* ParticlesToCut::instance = nullptr;

ParticlesToCut *ParticlesToCut::GetInstance(){
    if(instance == nullptr){
        std::cout << "Instancing" << std::endl;
        instance = new ParticlesToCut();
    }

    return instance;
}

