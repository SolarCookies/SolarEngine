#pragma once
#include <string>
#include "../Windows/Window.h"
#include "../World/Camera.h"

// Forward declarations
class Actor;
class World;

// Base Component class
class Component {
public:
    Actor* owner = nullptr;
    Component() = default;
    virtual ~Component() = default;
    virtual void Construct() {}
    virtual void BeginPlay() {}
    virtual void Tick(float deltaTime, World* World) {}
	virtual void EndPlay() {}
	virtual void Render(VinceWindow* window, Camera* Cam) {}
    virtual void Draw() {};
	virtual std::string GetName() const { return name; }

    Actor* Owner;
protected:
	std::string name = "Component"; // Default name
};