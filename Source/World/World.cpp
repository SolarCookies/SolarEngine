#include "World.h"
#include "../Components/Lights/LightComponent.h"

LightComponent* World::GetLightSource()
{
	//get the first actor that has a LightComponent
	for (auto& actor : Actors) {
		if (actor->GetComponentByName("LightComponent")) {
			LightComponent* Light = dynamic_cast<LightComponent*>(actor->GetComponentByName("LightComponent"));
			if (Light) {
				return Light;
			}
			else {
				return nullptr;
			}
		}
	}
	return nullptr;
}
