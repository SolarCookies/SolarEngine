#include "DebugPack_FileBrowser.h"

#include "imgui.h"
#include "imgui_memory_editor.h"
#include <filesystem>
#include "../../../../../../GlobalSettings.h"
#include "../../../../FileSystem/GlobalPackages.h"
#include "../../../../FileDatabase.h"


#include <random>

//#include "../../../../FileSystem/CAFF/CAFF.h"

void DebugPack_FileBrowser::RenderFileBrowser()
{

	//File Browser Window
	if (ImGui::Begin("Debug Pack File Browser"))
	{
		if (1)
		{
			if (DPack->IsValid)
			{
				ImGui::Text("Current Path: ");
				ImGui::SameLine();
				if (ImGui::Button("DebugPack"))
				{
					if (PendingChange) {
						ImGui::OpenPopup("ChangeFilePopup");
					}
					else
					{
						CurrentCAFF = "";
						CurrentModel.clear();
						CurrentModelName = "";
						chunkTextures.clear();
					}
				}

				// This should be outside the button block, but inside the window
				if (ImGui::BeginPopup("ChangeFilePopup")) {
					ImGui::Text("Are you sure you want to patch this CAFF?");
					if (ImGui::Button("Yes")) {
						PendingChange = false;
						CurrentCAFF = "";
						CurrentModel.clear();
						CurrentModelName = "";
						chunkTextures.clear();
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("No")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Delete")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				if (1) //Was to check if pkg was valid but because we are in debug pack, it just has caffs (Also im lazy and dont want to remove all the tabs)
				{
					if (CurrentCAFF != "")
					{
						ImGui::SameLine();
						ImGui::Text("/");
						ImGui::SameLine();
						if (ImGui::Button(CurrentCAFF.c_str()))
						{
							CurrentAid = "";
						}

						ImGui::Text("Search: ");
						ImGui::SameLine();

						strncpy(Searchbuf, SearchTerm.c_str(), sizeof(Searchbuf) - 1);
						ImGui::InputText("##Search", Searchbuf, 256);
						SearchTerm = Searchbuf;


						ImGui::BeginChild("Scrolling");
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

						int currentcaffindex = std::stoi(CurrentCAFF.substr(CurrentCAFF.find_last_of(" ") + 1)) - 1;

						//for each chunk in VREF
						for (int i = 0; i < DPack->CAFFs[currentcaffindex].chunks.size(); i++)
						{

							//if search term is empty or chunk name contains search term
							if (SearchTerm == "" || DPack->CAFFs[currentcaffindex].chunks[i]->Name.find(SearchTerm) != std::string::npos)
							{
								//Render List Item
								DPack->CAFFs[currentcaffindex].chunks[i]->RenderListItem();
							}

						}

						ImGui::PopStyleVar();

						ImGui::EndChild();
					}

					///No CAFF Selected
					else {
						//add scrollable area
						ImGui::BeginChild("Scrolling");

						//add padding
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

						for (int i = 0; i < DPack->caffNumber; i++)
						{
							ImGui::PushID(i);
							ImGui::BeginGroup();

							if (ImGui::IsItemHovered())
							{
								ImGui::BeginTooltip();
								ImGui::Text("CAFF %d", i + 1);
								ImGui::EndTooltip();
							}
							bool hasChanges = false;
							if (DPack->CAFFs[i].HasPendingChanges()) {
								hasChanges = true;
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.4f, 0.0f, 1.0f));
								//set text to black for better visibility
								ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
							}
							std::string CAFFName = FileDatabase::GetName(DPack->CAFFs[i].FileHash);
							if (CAFFName == "") CAFFName = "CAFF " + std::to_string(i + 1);
							if (ImGui::Button(CAFFName.c_str())) {
								DPack->CAFFs[i].Open();
								ImGui::OpenPopup("caffpopup");
							}
							if (hasChanges) {
								ImGui::PopStyleColor(4);
							}
							if (ImGui::BeginPopup("caffpopup")) {
								DPack->CAFFs[i].RenderListClick(CurrentCAFF, i);
								if (ImGui::Button("Rename")) {
									ImGui::OpenPopup("renamecaffpopup");
								}
								if (ImGui::BeginPopup("renamecaffpopup")) {
									static char newname[256] = "";
									if (newname[0] == '\0')
									{
										//set newname to CAFFName
										strncpy(newname, CAFFName.c_str(), sizeof(newname) - 1);
									}

									ImGui::InputText("New Name", newname, 256);
									if (ImGui::Button("Confirm")) {
										FileDatabase::AddEntry(DPack->CAFFs[i].FileHash, std::string(newname));
										ImGui::CloseCurrentPopup();
									}
									ImGui::SameLine();
									if (ImGui::Button("Cancel")) {
										ImGui::CloseCurrentPopup();
									}
									ImGui::SameLine();
									if (ImGui::Button("Delete")) {
										FileDatabase::RemoveEntry(DPack->CAFFs[i].FileHash);
										ImGui::CloseCurrentPopup();
									}
									ImGui::EndPopup();
								}
								ImGui::EndPopup();
							}

							ImGui::EndGroup();
							ImGui::PopID();
						}

						ImGui::PopStyleVar();

						ImGui::EndChild();
					}
				}

			}

		}
	}
	ImGui::End();

	if (FileViewBuffer.size() > 0)
	{
		static MemoryEditor hex_edit;
		if (!hex_edit.DrawWindow("Hex Editor", FileViewBuffer.data(), FileViewBuffer.size(), CurrentAid.c_str()))
		{

		}
	}

	RenderTexture();
}

void DebugPack_FileBrowser::RenderTexture() {
	if (PreviewTexture) {
		ImGui::Begin("Texture Preview", &ShowImageWindow, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Texture Preview: %s", CurrentAid.c_str());
		ImVec2 AvailableRegion = ImGui::GetContentRegionAvail();
		ImGui::Image((ImTextureID)(intptr_t)PreviewTexture->ID, AvailableRegion);
		ImGui::End();
	}
}

void DebugPack_FileBrowser::InitRFB() {
	LoadSettings();
	PackDirectory = DebugPackDicrectory + "\\debug_pack.bin";
	std::vector<unsigned char> fake;
	DPack = std::make_unique<DebugPack>(fake, PackDirectory);
}
