#include "MovementComponent.h"
#include "../../Actors/Actor.h"
#include "RigidBodyComponent.h"

void MovementComponent::Tick(float deltaTime, World* World)
{
	Time += deltaTime;
	//If onwner has a rigidbody, add volocity rather than position
	if (owner->GetComponentByName("RigidBodyComponent") != nullptr)
	{
		RigidBodyComponent* rigidBody = static_cast<RigidBodyComponent*>(owner->GetComponentByName("RigidBodyComponent"));
		if (rigidBody != nullptr)
		{
			rigidBody->ApplyForce(glm::vec3(sin(Time), 0.0f, 0.0f) * deltaTime * 10000000.0f);
		}
	}
	else {
		Time += deltaTime;
		owner->AddWorldPosition(glm::vec3(sin(Time), 0.0f, 0.0f) * deltaTime * 2.0f);
	}
	Component::Tick(deltaTime, World);
}
