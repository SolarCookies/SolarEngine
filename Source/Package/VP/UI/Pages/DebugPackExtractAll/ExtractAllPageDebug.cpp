#include "ExtractAllPageDebug.h"
#include "../../../FileSystem/DebugPack/DebugPack.h"
#include "../../../../../GlobalSettings.h"

void ExtractPageDebug::render(GUI& gui)
{
	LoadSettings();
	ExtractAll = true;
	
	ImGui::Begin("Welcome");
	//center the text
	ImGui::SetWindowFontScale(1.0f);
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("With this menu you can extract all the files inside the debug pack").x) * 0.5f);
	ImGui::Text("With this menu you can extract all the files inside the debug pack");
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Please select a export path for the files").x) * 0.5f);
	ImGui::Text("Please select a export path for the files");
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Select Path").x) * 0.5f);
	//Add button to select the path
	if (ImGui::Button("Select Path")) {
		Path = Walnut::OpenFileDialog::OpenFolder();
	}
	// Display the selected path
	if (Path.empty()) {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(std::string("No path selected.").c_str()).x) * 0.5f);
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No path selected.");
		goto End;
	}

	//Check to make sure the path is empty
	if (!std::filesystem::is_empty(Path)) {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(std::string("The selected path is not empty. You may want to select a empty folder.").c_str()).x) * 0.5f);
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "The selected path is not empty. You may want to select a empty folder.");
	}
	else {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(std::string("Selected Path: " + Path).c_str()).x) * 0.5f);
		ImGui::Text(("Selected Path: " + Path).c_str());
	}
	//Add a text input for a filter for the file, any file with this string in the name will be extracted
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Filter (optional)").x) * 0.5f);
	ImGui::Text("Filter (optional)");
	ImGui::SetCursorPosX(((ImGui::GetWindowWidth() - ImGui::CalcTextSize(filter).x) * 0.5f)*0.35f);
	ImGui::InputText("##filter", filter, sizeof(filter)); // Pass the char array and its size

	//Add a checkbox to export only specific file types (like .png, .jpg, .tga, .obj, .fbx, .mp4, .wav, .mp3, .ogg, .txt, .json, .xml)
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Export Textures as DDS and PNG").x) * 0.5f);
	ImGui::Checkbox("Export Textures as DDS and PNG", &ExportFileTypes);

	
	//If the path is valid, the rest will run
	// Add a button to continue and make it green
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Extract").x) * 0.5f);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
	if (ImGui::Button("Extract")) {
		PKGIndex = 0; //start extracting from the first .pkg file
	}
	ImGui::PopStyleColor();

	if (PKGIndex != -1) {
		std::string PackDirectory = "";
		std::unique_ptr<DebugPack> DPack;

		LoadSettings();
		PackDirectory = DebugPackDicrectory + "\\debug_pack.bin";
		std::vector<unsigned char> fake;
		DPack = std::make_unique<DebugPack>(fake, PackDirectory);
		
		for(size_t i = 0; i < DPack->CAFFs.size(); i++)
		{
			CAFF& caff = DPack->CAFFs[i];
			caff.Open();

			for (size_t j = 0; j < caff.chunks.size(); j++) {
				ExportInfo chunkfile = caff.chunks[j]->ExportChunk();
				//Export chunk to path C:\Projects\SolarEngine\Assets\Dump\chunkfile.name + .chunkfile.extension
				//If filter is not empty, only export files that contain the filter string in the name
				std::string DumpPath = "C:\\Projects\\SolarEngine\\Assets\\Dump";
				if (strlen(filter) > 0) {
					std::string filterStr(filter);
					if (chunkfile.Name.find(filterStr) != std::string::npos) {
						goto locexport; //found the filter string in the name, export the file
					}
					else {
						continue; //didn't find the filter string in the name, skip the file
					}
				}
locexport:
				if (!ExportFileTypes) {
					std::string fullpath1 = Path + "\\" + "PKG_" + std::to_string(PKGIndex) + "CAFF_" + std::to_string(i + 1) + chunkfile.Name + "." + "vdat";
					std::string fullpath2 = Path + "\\" + "PKG_" + std::to_string(PKGIndex) + "CAFF_" + std::to_string(i + 1) + chunkfile.Name + "." + "vgpu";
					//create directories if they don't exist
					std::filesystem::create_directories(std::filesystem::path(fullpath1).parent_path());

					//export .vdat and .vgpu files of chunk rather then specialized formats like .png, .dds, .jpg, .tga, .obj, .fbx, .mp4, .wav, .mp3, .ogg, .txt, .json, .xml
					std::vector<unsigned char>& fileData1 = caff.chunks[j]->VDAT;
					std::vector<unsigned char>& fileData2 = caff.chunks[j]->VGPU;

					std::ofstream outfile1(fullpath1, std::ios::binary);
					outfile1.write((char*)fileData1.data(), fileData1.size());
					outfile1.close();
					std::ofstream outfile2(fullpath2, std::ios::binary);
					outfile2.write((char*)fileData2.data(), fileData2.size());
					outfile2.close();

				}
				else {
					std::string fullpath = DumpPath + "\\" + chunkfile.Name + "." + chunkfile.Extension;
					//create directories if they don't exist
					std::filesystem::create_directories(std::filesystem::path(fullpath).parent_path());
					if (chunkfile.SaveFunction != nullptr) {
						chunkfile.SaveFunction(fullpath);
						continue; //skip the rest of the loop
					}

					std::ofstream outfile(fullpath, std::ios::binary);
					outfile.write((char*)chunkfile.Data.data(), chunkfile.Data.size());
					outfile.close();
				}
			}
			
		}
		DPack->Destroy();

		PKGIndex = -1; //reset to -1 to indicate we're done

	}

End:
	ImGui::End();
};
