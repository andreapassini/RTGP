#pragma once

//#include <bullet/btBulletDynamicsCommon.h>
#include "../bullet/btBulletDynamicsCommon.h"
#include "../glm/glm.hpp"

enum shapes{BOX, SPHERE};

class Physics
{
private:
    /* data */
public:
    btDiscreteDynamicsWorld* dynamicsWorld;
    btAlignedObjectArray<btCollisionShape*> collisionShapes;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;

    btBroadphaseInterface* overlappingPairCache;

    btSequentialImpulseConstraintSolver* solver;

    Physics(/* args */){
        this->collisionConfiguration = new btDefaultCollisionConfiguration();
        this->dispatcher = new btCollisionDispatcher(this->collisionConfiguration);
        this->overlappingPairCache = new btDbvtBroadphase();
        this->solver = new btSequentialImpulseConstraintSolver();

        this->dynamicsWorld = new btDiscreteDynamicsWorld(this->dispatcher,
                                                            this->overlappingPairCache,
                                                            this->solver,
                                                            this->collisionConfiguration);

        this->dynamicsWorld->setGravity(btVector3(0.0f, -9.82f, 0.0f));
    };
    //~Physics();

    btRigidBody* createRigidBody(int type, glm::vec3 pos, glm::vec3 size, glm::vec3 rot, float m, float friction, float restitution)
    {
        btCollisionShape* cShape = NULL;
        btVector3 position = btVector3(pos.x, pos.y, pos.z);
        btQuaternion rotation;
        rotation.setEuler(rot.x, rot.y, rot.z);

        if(type == BOX){
            btVector3 dim = btVector3(size.x, size.y, size.z);
            cShape = new btBoxShape(dim);
        } else if (type == SPHERE){
            cShape = new btSphereShape(size.x);
        }

        this->collisionShapes.push_back(cShape);

        btTransform objTransform;
        objTransform.setIdentity();
        objTransform.setRotation(rotation);
        objTransform.setOrigin(position);

        btScalar mass = m;
        // Set the mass at 0 if we want it to be STATIC
        bool isDynamic = (mass != 0.0f);

        btVector3 localInertia(0.0f, 0.0f, 0.0f);
        if(isDynamic){
            cShape->calculateLocalInertia(mass, localInertia);
        }

        btDefaultMotionState* motionState = new btDefaultMotionState(objTransform);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, cShape, localInertia);
        rbInfo.m_friction = friction;
        rbInfo.m_restitution = restitution;

        // the virtual sphere touches only in 1 point => no length for the friction

        if(type == SPHERE){
            rbInfo.m_angularDamping = 0.3f;
            rbInfo.m_rollingFriction = 0.3f;
        }

        btRigidBody* body = new btRigidBody(rbInfo);

        this->dynamicsWorld->addRigidBody(body);

        return body;
    };

    void Clear()
    {
        for(size_t i = this->dynamicsWorld->getNumCollisionObjects()-1; i >= 0; i--)
        {
            btCollisionObject* obj = this->dynamicsWorld->getCollisionObjectArray()[i];

            btRigidBody* body = btRigidBody::upcast(obj);
            if(body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            this->dynamicsWorld->removeCollisionObject(obj);
            delete obj;
        }

        delete this->dynamicsWorld;
        delete this->solver;
        delete this->overlappingPairCache;
        delete this->dispatcher;
        delete this->collisionConfiguration;
        this->collisionShapes.clear();

    }


};
