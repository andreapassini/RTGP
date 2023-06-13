#pragma once

// Singleton class
// ref: https://refactoring.guru/design-patterns/singleton/cpp/example#example-0
// Naive approach not thread safe

class PhysicsParameters
{
private:
    PhysicsParameters* instance;

    PhysicsParameters(/* args */);
    ~PhysicsParameters();

    float dampingForce;
    float fixedTimeStep;

public:
    /**
     * Singletons should not be cloneable.
     */
    PhysicsParameters(PhysicsParameters const&) = delete;
     /**
     * Singletons should not be assignable.
     */
    PhysicsParameters& operator=(PhysicsParameters const&) = delete;

    PhysicsParameters* GetInstance()
    {
        /**
         * This is a safer way to create an instance. instance = new Singleton is
         * dangeruous in case two instance threads wants to access at the same time
         */
        if(instance==nullptr){
            instance = new PhysicsParameters();
        }
        return instance;
    }

    void SetParameters(float fixedFramerate, float damping ){
        this->fixedTimeStep = 1.0f / fixedFramerate;
        this->dampingForce = damping;

    }
};