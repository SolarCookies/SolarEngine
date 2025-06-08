#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include "../Components/Component.h"
//#include "rpc.h"
#include "../Windows/Window.h"
#include "../World/Camera.h"

class World;

// Simple Actor class with built-in transform and component support
class Actor {
public:

    Actor();
    virtual ~Actor();

    bool TestMovement;

    // Unreal-style lifecycle
    virtual void Construct();
    virtual void BeginPlay();
    virtual void Tick(float deltaTime, World* world);
    virtual void Render(VinceWindow* Window, Camera* Cam);
	virtual void EndPlay();
    //virtual void Destroy();
    
	virtual std::string GetClassName() const { return className; }
    //const UUID GetID() const { return GUID; }
	//void SetID(const UUID& id) { GUID = id; }


    // Transform utilities
    glm::mat4 GetTransform() const {
        return WorldTransform;
    }

    glm::vec3 GetWorldPosition() const {
        return glm::vec3(WorldTransform[3]); // Extract translation from the matrix
	}

    void SetWorldPosition(glm::vec3 Position) {
		WorldTransform = glm::mat4(1.0f); // Reset to identity
        WorldTransform = glm::translate(glm::mat4(1.0f), Position);
	}

    glm::vec3 GetWorldScale() const {
        return glm::vec3(glm::length(WorldTransform[0]), glm::length(WorldTransform[1]), glm::length(WorldTransform[2]));
    }

    void SetWorldScale(const glm::vec3& Scale) {
        WorldTransform = glm::scale(glm::mat4(1.0f), Scale);
    }

    glm::vec3 GetWorldRotation() const {
        // Extract rotation from the matrix (assuming no scaling)
        glm::mat3 rotationMatrix = glm::mat3(WorldTransform);
        return glm::eulerAngles(glm::quat_cast(rotationMatrix)); // Convert to Euler angles
	}

    void SetWorldRotation(const glm::vec3& Rotation) {
        WorldTransform = glm::rotate(WorldTransform, Rotation.x, glm::vec3(1, 0, 0)); // Pitch
        WorldTransform = glm::rotate(WorldTransform, Rotation.y, glm::vec3(0, 1, 0)); // Yaw
        WorldTransform = glm::rotate(WorldTransform, Rotation.z, glm::vec3(0, 0, 1)); // Roll

	}

    void AddWorldPosition(const glm::vec3& Position) {
		WorldTransform = glm::translate(WorldTransform, Position);
	}

    // Component management
    void AddComponent(std::shared_ptr<Component> component);

	// Get first component by Name
    Component* GetComponentByName(const std::string& name) const {
        for (const auto& comp : components) {
            if (comp->GetName() == name) {
				return comp.get();
            }
        }
		return nullptr;
    }

	// Get all components by Name
    std::vector<Component*> GetAllComponentsByName(const std::string& name) const {
        std::vector<Component*> foundComponents;
        for (const auto& comp : components) {
            if (comp->GetName() == name) {
                foundComponents.push_back(comp.get());
            }
        }
		return foundComponents;
	}

    Component* GetComponentByIndex(int index) const {
        if (index >= 0 && index < components.size()) {
            return components[index].get();
        }
        return nullptr;
	}

protected:
    std::vector<std::shared_ptr<Component>> components;
    bool hasBegunPlay = false;
	std::string className = "Actor";
    glm::mat4 WorldTransform = glm::mat4(1.0f); // World transform matrix

private:
   // UUID GUID;
};