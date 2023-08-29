#include <vector>
#include <utils/Particle.h>

class ParticlesToCut
{
private:
    ParticlesToCut();
    ~ParticlesToCut();
public:
    std::vector<Particle*> particles;

    static ParticlesToCut& instance(){
        static ParticlesToCut INSTANCE;
        return INSTANCE;
    }

    void CleanUp(){
        particles.clear();
    }
};