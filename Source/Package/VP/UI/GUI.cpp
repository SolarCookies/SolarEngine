#include "GUI.h"
#include "Pages/WelcomePage.h"
#include "../../../GlobalSettings.h"
#include "Pages/EditorSelectionPage.h"
#include "imgui.h" // Add this include for ImGui menu bar

void GUI::init()
{
	LoadSettings();
	if(RootDicrectory.empty())
	{
		CurrentPage = std::make_unique<WelcomePage>();
	}
	else {
		CurrentPage = std::make_unique<EditorPage>();
	}
	SaveSettings();
	HasInitialized = true;
}

void GUI::render()
{
    if (CurrentPage) CurrentPage->render(*this);
	return;

    // --- Add menu bar at the top of the window ---
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            // Example menu item, you can add more as needed
            if (ImGui::MenuItem("Exit"))
            {
                // You can handle exit logic here if needed
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Pages"))
        {
            if (ImGui::MenuItem("Welcome Page"))
            {
                CurrentPage = std::make_unique<WelcomePage>();
            }
            if(ImGui::MenuItem("Editor Selection"))
            {
                CurrentPage = std::make_unique<EditorPage>();
            }
            ImGui::EndMenu();
		}
        if (ImGui::BeginMenu("ImGui Debug"))
        {
            if (ImGui::MenuItem("Demo Menu"))
            {
				ShowImGuiDemoWindow = !ShowImGuiDemoWindow;
            }
            if( ImGui::MenuItem("Metrics"))
			{
				ShowMetricsWindow = !ShowMetricsWindow;
			}
			if (ImGui::MenuItem("Style Editor"))
			{
				ShowStyleEditor = !ShowStyleEditor;
			}
            
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    // --- End menu bar code ---

    if (ShowImGuiDemoWindow)
    {
        ImGui::ShowDemoWindow();
    }
    if (ShowMetricsWindow)
    {
        ImGui::ShowMetricsWindow();
    }
    if (ShowStyleEditor)
    {
        ImGui::Begin("Style Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::ShowStyleEditor();
        ImGui::End();
    }

    if (CurrentPage) CurrentPage->render(*this);
}
