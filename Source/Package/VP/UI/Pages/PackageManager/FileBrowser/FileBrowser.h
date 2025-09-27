#pragma once
#include "../../../../FileSystem/PKG/PKG.h"

#include "imgui.h"
#include "imgui_memory_editor.h"
#include <filesystem>
#include "../../../../../../GlobalSettings.h"
#include "../../../../FileSystem/GlobalPackages.h"
#include "../../../../FileDatabase.h"


#include <random>

//Enum for render mode
enum class RenderMode
{
	List,
	Grid
};

class FileBrowser
{
public:
	char Bundlepath[255] = { 0 };
	

	FileBrowser() {
		LoadSettings();
		for(int i = 0; i < 255; i++) {
			Bundlepath[i] = '\0';
		}
		for(int i = 0; i < PackagesDicrectory.length(); i++) {
			Bundlepath[i] = PackagesDicrectory[i];
		}
		std::string BundlesDicrectoryEnglish = BundlesDicrectory + "\\englishus.bnl";
	};
	~FileBrowser() {
	};
	
	std::string CurrentPKG = "";
	//PKG pkg;
	std::string CurrentCAFF = "";
	std::string CurrentAid = "";

	bool ShowImageWindow = false;

	bool UpdateImage = false;
	bool ShowRaw = false;

	char Searchbuf[255]{};
	std::string SearchTerm = "";

	std::unique_ptr <Package> pkg;

	bool FoundFiles = false;
	
	std::vector<std::string> files;
	std::vector<std::string> FileNames;

	void reloadPKG()
	{
		if (CurrentPKG != "")
		{
			//pkg::ReloadPKG(CurrentPKG, pkg);
			CurrentCAFF = "";
			CurrentAid = "";
			CurrentPKG = "";
			FoundFiles = false;

		}
	}

	void RenderFileBrowser(RenderMode r) {

		//File Browser Window
		if (ImGui::Begin("File Browser"))
		{
			if (r == RenderMode::List)
			{
				if (Bundlepath != nullptr || Bundlepath[0] != '\0' || Bundlepath != "")
				{
					ImGui::Text("Current Path: ");
					ImGui::SameLine();
					if (ImGui::Button(std::string(Bundlepath).substr(std::string(Bundlepath).find_last_of("\\") + 1).c_str()))
					{
						CurrentPKG = "";
						CurrentCAFF = "";
					}

					if (CurrentPKG != "")
					{
						ImGui::SameLine();
						ImGui::Text("/");
						ImGui::SameLine();
						if (ImGui::Button(CurrentPKG.c_str()))
						{
							CurrentCAFF = "";
						}

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
							for (int i = 0; i < pkg->CAFFs[currentcaffindex].chunks.size(); i++)
							{

								//if search term is empty or chunk name contains search term
								if (SearchTerm == "" || pkg->CAFFs[currentcaffindex].chunks[i]->Name.find(SearchTerm) != std::string::npos)
								{
									//Render List Item
									pkg->CAFFs[currentcaffindex].chunks[i]->RenderListItem();
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

							//To test add 10 dummy files
							for (int i = 0; i < pkg->Header.CAFFCount; i++)
							{
								ImGui::PushID(i);
								ImGui::BeginGroup();
								//ImGui::ImageButton((ImTextureID)0, ImVec2(50, 50));
								//ImGui::Image(m_Unknown_Thumbnail.GetDescriptorSet(), ImVec2(50, 50));
								//ImGui::SameLine();
								//center height
								//ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (50 - ImGui::GetTextLineHeight()) / 2);
								//i.caff
								//tool tip
								if (ImGui::IsItemHovered())
								{
									ImGui::BeginTooltip();
									ImGui::Text("CAFF %d", i + 1);
									ImGui::Text("PKG Offset: %d", pkg->CAFFEntries[i].Offset);
									ImGui::Text("PKG Size: %d", pkg->CAFFEntries[i].Size);
									ImGui::EndTooltip();
								}
								bool hasChanges = false;
								if (pkg->CAFFs[i].HasPendingChanges()) {
									hasChanges = true;
									ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.4f, 0.0f, 1.0f));
									//set text to black for better visibility
									ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
								}
								std::string CAFFName = FileDatabase::GetName(pkg->CAFFs[i].FileHash);
								if (CAFFName == "") CAFFName = "CAFF " + std::to_string(i + 1);
								if (ImGui::Button(CAFFName.c_str())) {
									ImGui::OpenPopup("caffpopup");
								}
								if (hasChanges) {
									ImGui::PopStyleColor(4);
								}
								if (ImGui::BeginPopup("caffpopup")) {
									pkg->CAFFs[i].RenderListClick(CurrentCAFF, i);
									if (ImGui::Button("Rename")) {
										ImGui::OpenPopup("renamecaffpopup");
									}
									if (ImGui::BeginPopup("renamecaffpopup")) {
										static char newname[256] = "";
										if(newname[0] == '\0')
										{
											//set newname to CAFFName
											strncpy(newname, CAFFName.c_str(), sizeof(newname) - 1);
										}

										ImGui::InputText("New Name", newname, 256);
										if (ImGui::Button("Confirm")) {
											FileDatabase::AddEntry(pkg->CAFFs[i].FileHash, std::string(newname));
											ImGui::CloseCurrentPopup();
										}
										ImGui::SameLine();
										if (ImGui::Button("Cancel")) {
											ImGui::CloseCurrentPopup();
										}
										ImGui::SameLine();
										if (ImGui::Button("Delete")) {
											FileDatabase::RemoveEntry(pkg->CAFFs[i].FileHash);
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

					///No PKG selected
					else {
						//add scrollable area
						ImGui::BeginChild("Scrolling");

						//add padding
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

						//for each file in directory
						if (!FoundFiles)
						{
							files = Walnut::OpenFileDialog::GetFilesInDirectory(Bundlepath);

							for (int i = 0; i < files.size(); i++)
							{
								FileNames.push_back(files[i].substr(files[i].find_last_of("\\") + 1));
							}
							FoundFiles = true;
						}

						for (int i = 0; i < files.size(); i++)
						{
							ImGui::BeginGroup();
							ImGui::PushID(i);

							//center height
							//ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (50 - ImGui::GetTextLineHeight()) / 2);

							std::string PKGName = FileDatabase::GetName(FileNames[i]);
							if (PKGName == "") PKGName = FileNames[i];

							if (ImGui::Button(PKGName.c_str()))
							{
								ImGui::OpenPopup("pkgpopup");
							}
							if (ImGui::BeginPopup("pkgpopup")) {
								if (ImGui::Button("Open")) {
									CurrentPKG = FileNames[i];
									std::cout << "Selected: " << CurrentPKG << std::endl;
									//read pkg
									std::vector<unsigned char> data;
									//open file and read to data
									std::ifstream file(files[i], std::ios::binary);
									if (file.is_open())
									{
										data = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
										file.close();
										if (pkg) pkg->Destroy();
										pkg = std::make_unique <Package>(data, files[i]);
									}
									else
									{
										std::cout << "Failed to open file: " << files[i] << std::endl;
									}
									
								}
								if (ImGui::Button("Rename")) {
									ImGui::OpenPopup("renamepkgpopup");
								}
								if(ImGui::BeginPopup("renamepkgpopup")) {
									static char newname[256] = "";
									if(newname[0] == '\0')
									{
										//set newname to PKGName
										strncpy(newname, PKGName.c_str(), sizeof(newname) - 1);
									}
									

									ImGui::InputText("New Name", newname, 256);
									if (ImGui::Button("Confirm")) {
										FileDatabase::AddEntry(FileNames[i], std::string(newname));
										ImGui::CloseCurrentPopup();
									}
									ImGui::SameLine();
									if (ImGui::Button("Cancel")) {
										ImGui::CloseCurrentPopup();
									}
									ImGui::SameLine();
									if (ImGui::Button("Delete")) {
										FileDatabase::RemoveEntry(FileNames[i]);
										ImGui::CloseCurrentPopup();
									}
									ImGui::EndPopup();
								}
								/*
								if (std::filesystem::exists("Backups\\PackageBundles\\" + files[i].substr(pkg->Path.find_last_of("\\") + 1)))
								{
									if (ImGui::Button("Restore")) {
										std::filesystem::remove(files[i]);
										std::filesystem::copy("Backups\\PackageBundles\\" + files[i].substr(pkg->Path.find_last_of("\\") + 1), files[i]);
										std::cout << "Restored: " << files[i] << std::endl;
										ImGui::CloseCurrentPopup();
									}
								}
								*/
								ImGui::EndPopup();
							}

							ImGui::PopID();
							ImGui::EndGroup();
						}

						ImGui::PopStyleVar();

						ImGui::EndChild();
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
				//if (GlobalSaveFunction) {
				//	GlobalSaveFunction(FileViewBuffer);
				//}
				//FileViewBuffer.clear();
			}
		}

		RenderTexture();
	}

	void RenderTexture()
	{
		if (PreviewTexture) {
			ImGui::Begin("Texture Preview", &ShowImageWindow, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text("Texture Preview: %s", CurrentAid.c_str());
			ImVec2 AvailableRegion = ImGui::GetContentRegionAvail();
			ImGui::Image((ImTextureID)(intptr_t)PreviewTexture->ID, AvailableRegion);
			ImGui::End();
		}
	}
};