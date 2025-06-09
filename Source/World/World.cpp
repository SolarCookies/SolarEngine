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

void World::RenderWorldOutliner()
{
	ImGui::Begin("WorldOutliner");
	for(auto& actor : Actors) {
		ImGui::Text(actor->ActorName.c_str());
		ImGui::SameLine();
		if(actor->Selected) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Selected");
		}
		else {
			if (ImGui::Button(("Select##" + actor->ActorName).c_str())) {
				// Select the actor in the editor
				if (CurrentlySelectedActor) {
					CurrentlySelectedActor->Selected = false;
				}
				CurrentlySelectedActor = actor.get();
				CurrentlySelectedActor->Selected = true;

			}
		}
		
	}
	ImGui::End();
}
