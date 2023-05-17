#include <vector>

class physicEngine
{
private:
    // Array of objects to apply physic to

public:
    bool applyWind;
    physicEngine(/* args */);
    ~physicEngine();

    void PhysicStep();
};

physicEngine::physicEngine(/* args */)
{
}

physicEngine::~physicEngine()
{
}
