#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include "../Components/Component.h"
//#include "rpc.h"
#include "../Windows/Window.h"
#include "../World/Camera.h"
#include <imgui.h>

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

    void UpdateTransform() {
        WorldTransform = glm::translate(glm::mat4(1.0f), WorldPosition)
            * glm::mat4_cast(WorldRotation)
            * glm::scale(glm::mat4(1.0f), WorldScale);
    }

    // Transform utilities
    glm::mat4 GetTransform() const {
        return WorldTransform;
    }

    glm::vec3 GetWorldPosition() const {
        return WorldPosition; // Extract translation from the matrix
	}

    glm::vec3 GetWorldScale() const {
        return WorldScale;
    }

    glm::quat GetWorldRotation() const {
		return WorldRotation;
	}

    void SetWorldPosition(const glm::vec3& pos) { WorldPosition = pos; UpdateTransform(); }
    void SetWorldScale(const glm::vec3& scale) { WorldScale = scale; UpdateTransform(); }
    void SetWorldRotation(const glm::vec3& eulerRadians) { WorldRotation = glm::quat(eulerRadians); UpdateTransform(); }
    void SetWorldRotation(const glm::quat& quat) { WorldRotation = quat; UpdateTransform(); }

    void AddWorldPosition(const glm::vec3& Position) {
        WorldPosition += Position;
		UpdateTransform();
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

    void RenderDetails() {
		
        if (ImGui::SliderFloat3("Position", &WorldPosition.x, -100.0f, 100.0f)) {
			UpdateTransform();
        }
        if (ImGui::SliderFloat3("Rotation", &WorldRotation.x, -3.14f, 3.14f)) {
        //    UpdateTransform();
			Log("Rotation doesn't work yet, need to convert to quaternion (Pain)");
		}
        if (ImGui::SliderFloat3("Scale", &WorldScale.x, 0.1f, 10.0f)) {
            UpdateTransform();
        }
    }

	bool Selected = false;
    std::vector<std::string> ActorTags;
    std::string ActorName = "No Name";
protected:
    std::vector<std::shared_ptr<Component>> components;
    bool hasBegunPlay = false;
	std::string className = "Actor";
    glm::mat4 WorldTransform = glm::mat4(1.0f); // World transform matrix

    glm::vec3 WorldPosition = glm::vec3(0.0f);
    glm::vec3 WorldScale = glm::vec3(1.0f);
    glm::quat WorldRotation = glm::quat(glm::vec3(0.0f));


private:
   // UUID GUID;
};