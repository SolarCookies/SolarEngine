#pragma once
#include "../Actor.h"
#include "../../Components/StaticMeshComponent.h"
#include "../Mesh.h"
#include "../Windows/Log.hpp"

class AStaticMesh : public Actor
{

public:
	AStaticMesh(std::string OBJPath, std::string ShaderName)
		: Actor()
	{
		auto staticMeshComponent = std::make_shared<StaticMeshComponent>(OBJPath, ShaderName);
		AddComponent(staticMeshComponent);
	}
	virtual ~AStaticMesh() {}
	void Tick(float deltaTime, World* world) override
	{
		Actor::Tick(deltaTime, world);

	}

	
	
protected:

private:
	
};
