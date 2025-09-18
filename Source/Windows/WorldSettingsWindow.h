#pragma once
#include "../World/Camera.h"
#include "../World/World.h"
#include "../Utils/JoltHelpers.h"

#include "../Actors/Lights/APointLight.h"
#include "../Actors/Meshes/AStaticMesh.h"

#include "../Components/Movement/MovementComponent.h"
#include "../Components/Movement/RigidBodyComponent.h"

#include "../Components/Meshes/GatorMeshComponent.h"

class WorldSettingsWindow
{
	public:
	WorldSettingsWindow() = default;
	~WorldSettingsWindow() = default;
	void Draw(World& world, VinceWindow& Window, PhysicsSystem& physics_system) {
		{
			ImGui::Begin("World Settings");
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

			//gravity slider
			float gravity = physics_system.GetGravity().GetY();
			if (ImGui::DragFloat("Gravity", &gravity, 0.1f, -20.0f, 20.0f)) {
				//Set the gravity of the physics system
				physics_system.SetGravity(JPH::Vec3(0, gravity, 0));
			}
			if (ImGui::Button("Reset Dragon")) {
				std::vector<Actor*> actors = world.GetAllActorsWithTag("Dragon");
				for (Actor* actor : actors) {
					if (actor) {
						RigidBodyComponent* rb = dynamic_cast<RigidBodyComponent*>(actor->GetComponentByIndex(1));
						rb->SetBodyPosition({ 0,0,0 });
						actor->SetWorldPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					}
				}

			}
			//for each actor in the world, display its name and transform
			ImGui::Text("Actors in World: %d", world.GetActorCount());

			ImGui::End();
		}
	}
};
