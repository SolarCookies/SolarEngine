#include "MovementComponent.h"
#include "../Actor.h"

void MovementComponent::Tick(float deltaTime, World* World)
{
	Time += deltaTime;
	owner->AddWorldPosition(glm::vec3(sin(Time), 0.0f, 0.0f) * deltaTime * 2.0f);
	Component::Tick(deltaTime, World);
}
