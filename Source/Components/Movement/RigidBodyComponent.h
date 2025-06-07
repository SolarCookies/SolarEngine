#pragma once
#include "Jolt/Jolt.h"
#include "Jolt/Physics/Body/Body.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "../Component.h"
#include "../../Windows/Log.hpp"

class World; // Forward declaration for World class

class RigidBodyComponent : public Component {
public:
    RigidBodyComponent(JPH::PhysicsSystem* physicsSystem, const JPH::BodyCreationSettings& settings)
        : mPhysicsSystem(physicsSystem)
    {
        if (!mPhysicsSystem) {
            throw std::runtime_error("PhysicsSystem pointer is null in RigidBodyComponent");
        }
        if (settings.GetShape() == nullptr) {
            throw std::runtime_error("BodyCreationSettings shape is null in RigidBodyComponent");
        }
        auto& bodyInterface = mPhysicsSystem->GetBodyInterface();
        mBodyID = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::Activate);
		name = "RigidBodyComponent";

    }

    ~RigidBodyComponent() override {
        if (mPhysicsSystem)
            mPhysicsSystem->GetBodyInterface().RemoveBody(mBodyID);
    }

    void Tick(float deltaTime, World* world) override {
        if (!mPhysicsSystem) {
            throw std::runtime_error("PhysicsSystem pointer is null in RigidBodyComponent::Tick");
        }
        else {
            auto& bodyInterface = mPhysicsSystem->GetBodyInterface();
            JPH::Vec3 loc = bodyInterface.GetCenterOfMassPosition(mBodyID);

			//Update the actor's position based on the rigid body position
			glm::vec3 pos = glm::vec3(loc.GetX(), loc.GetY(), loc.GetZ());
            owner->SetWorldPosition(pos);
        }
    }

    void SetBodyPosition(const glm::vec3& position) {
        if (!mPhysicsSystem) {
            throw std::runtime_error("PhysicsSystem pointer is null in RigidBodyComponent::SetBodyPosition");
        }
        auto& bodyInterface = mPhysicsSystem->GetBodyInterface();
        JPH::Vec3 joltPosition(position.x, position.y, position.z);
        bodyInterface.SetLinearVelocity(mBodyID, {0, 0, 0});
		bodyInterface.SetPosition(mBodyID, joltPosition, JPH::EActivation::Activate);
	}

    void ApplyForce(const glm::vec3& force) {
        if (!mPhysicsSystem) {
            throw std::runtime_error("PhysicsSystem pointer is null in RigidBodyComponent::ApplyForce");
        }
        auto& bodyInterface = mPhysicsSystem->GetBodyInterface();
        JPH::Vec3 joltForce(force.x, force.y, force.z);
		bodyInterface.AddForce(mBodyID, joltForce, JPH::EActivation::Activate);
	}

    JPH::BodyID GetBodyID() const { return mBodyID; }

private:
    JPH::PhysicsSystem* mPhysicsSystem = nullptr;
    JPH::BodyID mBodyID;
};