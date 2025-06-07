#pragma once
#include "Actor.h"
//#include <rpc.h>
#include "VinceWindow.h"

class LightComponent;

class World {

public:
	Actor* GetActorOfClass(const std::string& ClassName) {
		for (auto& actor : Actors) {
			if (actor->GetClassName() == ClassName) {
				return actor.get();
			}
		}
		return nullptr;
	}

	std::vector<Actor*> GetAllActorsOfClass(const std::string& ClassName) {
		std::vector<Actor*> FoundActors;
		for (auto& actor : Actors) {
			if (actor->GetClassName() == ClassName) {
				FoundActors.push_back(actor.get());
			}
		}
		return FoundActors;
	}

	void AddActor(Actor& actor) {
		Actors.push_back(std::unique_ptr<Actor>(&actor));
	}

	void Render(VinceWindow* window, Camera* Cam) {
		for (auto& actor : Actors) {
			actor->Render(window, Cam);
		}
	}

	void ConstructWorld() {
		for (auto& actor : Actors) {
			actor->Construct();
		}
	}

	void TickWorld(float DeltaTime) {
		for (auto& actor : Actors) {
			actor->Tick(DeltaTime, this);
		}
	}

	LightComponent* GetLightSource();

	uint32_t GetActorCount() {
		return static_cast<uint32_t>(Actors.size());
	}

private:
	std::vector<std::unique_ptr<Actor>> Actors;

};