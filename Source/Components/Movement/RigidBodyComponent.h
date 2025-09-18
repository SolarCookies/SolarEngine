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
        bodyInterface.SetLinearVelocity(mBodyID, { 0, 0, 0 });
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

    void RenderDetails() override {
        // Implement collsion settings and physics properties here "Aka: mass, friction, restitution, etc."
        ImGui::Text("Body ID: %u", mBodyID.GetIndex());
        ImGui::Text("Physics System: %p", mPhysicsSystem);
        if (mPhysicsSystem) {
            auto& bodyInterface = mPhysicsSystem->GetBodyInterface();
            //velocity
            JPH::Vec3 velocity = bodyInterface.GetLinearVelocity(mBodyID);
            ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity.GetX(), velocity.GetY(), velocity.GetZ());
            //bodyInterface.GetGravityFactor(mBodyID);
            float Gravity = bodyInterface.GetGravityFactor(mBodyID);
            if (ImGui::DragFloat("Gravity Factor", &Gravity, 0.01f, 0.0f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
                bodyInterface.SetGravityFactor(mBodyID, Gravity);
            }

        }
        else {
            ImGui::Text("Physics System: NULL");
        }

    }

    void DebugBodyShape() {
        // Draw the body shape in the world  
        if (!mPhysicsSystem) {
            throw std::runtime_error("PhysicsSystem pointer is null in RigidBodyComponent::DebugBodyShape");
        }
        auto& bodyInterface = mPhysicsSystem->GetBodyInterface();
        JPH::RefConst<JPH::Shape> shapeRef = bodyInterface.GetShape(mBodyID);
        const JPH::Shape* shape = shapeRef.GetPtr(); // Use GetPtr() to retrieve the raw pointer  

        if (shape) {
            // Implement shape rendering logic here  
            // For example, you could use a debug renderer to visualize the shape

        }
    }

    

    JPH::BodyID GetBodyID() const { return mBodyID; }

    
private:
    JPH::PhysicsSystem* mPhysicsSystem = nullptr;
    JPH::BodyID mBodyID;

};