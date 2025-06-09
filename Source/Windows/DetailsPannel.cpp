#include "DetailsPannel.h"

void DetailsWindow::RenderDetailsWindow(World& world)
{
	ImGui::Begin("Details");
	if (world.CurrentlySelectedActor) {
		world.CurrentlySelectedActor->RenderDetails();
	}
	ImGui::End();
}
