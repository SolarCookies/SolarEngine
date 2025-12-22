#include "DetailsPannel.h"

void DetailsWindow::RenderDetailsWindow(World& world)
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.0f, 0.2f, 0.5f));
	ImGui::Begin("Details");
	if (world.CurrentlySelectedActor) {
		world.CurrentlySelectedActor->RenderDetails();
	}
	ImGui::End();
	ImGui::PopStyleColor();
}
