#pragma once
#include "../Component.h"

class Actor;

class MovementComponent : public Component
{
	public:
	MovementComponent() = default;
	~MovementComponent() override = default;
	void Tick(float deltaTime, World* World) override;

private:
	float Time;
};