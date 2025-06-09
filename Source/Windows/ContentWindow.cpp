#include "ContentWindow.h"

void ContentWindow::RenderContentWindow()
{
	ImGui::Begin("Content");
	ImGui::Text("This is the content window.");
	ImGui::End();
}
