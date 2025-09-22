#pragma once
#include "../File.h"
#include "../../Utils/OpenFIleDialog.h"
#include "../../../../GlobalSettings.h"
#include <ctime>

#pragma pack(push, 1)
struct ChunkInfoSection {
	int ID;
	int Offset;
	int Size;
	char Type;
	char unk;
};
#pragma pack(pop)


struct ChunkInfo {
	bool HasGpu = false;
	ChunkInfoSection VDat;
	ChunkInfoSection VGpu;
	int Offset1;
	int Offset2;
};

struct ExportInfo {
	std::string Name;
	std::string Extension;
	std::vector<unsigned char> Data;
	std::function<void(const std::string& path)> SaveFunction; //Function to call when saving the exported file this is used to overide the save funtionality ie use stb_image_write instead of raw file write
};

class Chunk : public File {
protected:
	std::vector<unsigned char> UpdatedVDAT;
	std::vector<unsigned char> UpdatedVGPU;
public:
	std::vector<unsigned char> VDAT;
	std::vector<unsigned char> VGPU;
	int FileIndex = 0; //Index of the chunk in the vref
	bool PendingUpdate = false; //If true, the chunk has been modified and needs to be saved back to the archive
	std::string Name;
	std::string NameWithoutMetadata;
	time_t FileTimestamp = 0; //Unix Time of the file, found in the name of the chunk, "aid_action_pinata_abilities_actoranimal,1185459620,2.54
	float FileType = 0.0; // found at the end of the chunk name, seems to be versioning?
	ChunkInfo info;

	Chunk() : File() {};
	Chunk(const std::vector<unsigned char>& rawFile) : File(rawFile) {};
	Chunk(const std::vector<unsigned char>& rawVDAT, const std::vector<unsigned char>& rawVGPU, bool& IsBig, std::string& name, ChunkInfo& Info);
	
	void LoadFile(const std::vector<unsigned char>& rawFile) override {

	}

	void SaveFile(const std::vector<unsigned char>& data) {
		switch (FileViewType) {
			case 0:
				UpdatedVDAT = data;
				PendingUpdate = true;
				break;
			case 1:
				UpdatedVGPU = data;
				PendingUpdate = true;
				break;
		}
	}

	virtual ExportInfo ExportChunk() { //Gets chunk as a common asset format like a dds or fbx returns empty if unknown or non asset file
		return ExportInfo();
	}

	virtual void RenderListHover() { //Used when drawing imgui list entry hover
		ImGui::BeginTooltip();
		ImGui::Text("ID: %d", info.VDat.ID);
		ImGui::Text("VDAT Offset: %d", info.VDat.Offset);
		ImGui::Text("VDAT Size: %d", info.VDat.Size);
		if (info.HasGpu)
		{
			ImGui::Text("VGPU Offset: %d", info.VGpu.Offset);
			ImGui::Text("VGPU Size: %d", info.VGpu.Size);
		}
		ImGui::Text("File Type: %.2f", FileType);
		//convert timestamp to human readable format
		if (FileTimestamp != 0) {
			char buffer[80];
			tm* timeinfo = localtime(&FileTimestamp);
			strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
			ImGui::Text("OG Timestamp: %s", buffer);
		}
		else {
			ImGui::Text("Timestamp: N/A");
		}
		ImGui::EndTooltip();
	}

	virtual void RenderListClick() { //Menu that pops up when clicking on the list entry
		if (ImGui::Button("View VDAT")) {
			FileName = NameWithoutMetadata + ".vdat";
			FileViewType = 0;
			if (PendingUpdate) {
				FileViewBuffer = UpdatedVDAT;
			}
			else {
				FileViewBuffer = VDAT;
			}
			GlobalSaveFunction = [this](const std::vector<unsigned char>& data) { this->SaveFile(data); };
		}
		ImGui::SameLine();
		if (ImGui::Button("Export VDAT")) {
			bool saved = Walnut::OpenFileDialog::AskSaveFile(VDAT, Name + ".vdat");
		}
		ImGui::SameLine();
		if (ImGui::Button("Replace VDAT")) {
			//select Patch file
			std::string PatchFilePath = Walnut::OpenFileDialog::OpenFile("VDAT Files\0*.vdat\0\0");
			//Open file and read it into UpdatedVDAT
			if (!PatchFilePath.empty()) {
				std::ifstream file(PatchFilePath, std::ios::binary);
				if (file) {
					UpdatedVDAT = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
					file.close();
					PendingUpdate = true;
				}
			}
		}
		if (info.HasGpu)
		{
			if (ImGui::Button("View VGPU")) {
				FileName = NameWithoutMetadata + ".vgpu";
				FileViewType = 1;
				if (PendingUpdate) {
					FileViewBuffer = UpdatedVGPU;
				}
				else {
					FileViewBuffer = VGPU;
				}
				GlobalSaveFunction = [this](const std::vector<unsigned char>& data) { this->SaveFile(data); };
			}
			ImGui::SameLine();
			if (ImGui::Button("Export VGPU")) {
				bool saved = Walnut::OpenFileDialog::AskSaveFile(VGPU, Name + ".vgpu");
			}
			ImGui::SameLine();
			if (ImGui::Button("Replace VGPU")) {
				//select Patch file
				std::string PatchFilePath = Walnut::OpenFileDialog::OpenFile("VGPU Files\0*.vgpu\0\0");
				//Open file and read it into UpdatedVGPU
				if (!PatchFilePath.empty()) {
					std::ifstream file(PatchFilePath, std::ios::binary);
					if (file) {
						UpdatedVGPU = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
						file.close();
						PendingUpdate = true;
					}
				}
			}
		}
	}

	virtual void RenderListItem() { //Used when drawing imgui list entry
		ImGui::PushID(info.VDat.ID);
		ImGui::BeginGroup();

		// Style the button to look like a flat listbox entry
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f); // Less rounded corners
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); // Add border
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4)); // More padding

		// If pending update, override colors for emphasis
		if (PendingUpdate) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.25f, 0.0f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.3f, 0.1f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.2f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			if (info.HasGpu) {
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Border color
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 1.0f, 1.0f)); // Border color
			}
		}
		else {
			// Set colors based on Data or GPU
			if (info.HasGpu) {
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Border color
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 1.0f, 1.0f)); // Border color
			}
		}
		if (ImGui::Button(NameWithoutMetadata.c_str(), ImVec2(-1, 0))) { // Full width
			ImGui::OpenPopup("popup");
		}
		if (ImGui::IsItemHovered()) {
			RenderListHover();
		}
		if (PendingUpdate) {
			// Pop Color Styles
			ImGui::PopStyleColor(5);
		}
		else {
			ImGui::PopStyleColor(1);
		}

		ImGui::PopStyleVar(3);

		if (ImGui::BeginPopup("popup")) {
			RenderListClick();
			ImGui::EndPopup();
		}
		ImGui::EndGroup();
		ImGui::PopID();
	}

	virtual void RenderDetails() {} //Used when drawing imgui details panel for editing the file or displaying info

	void Destroy() override {
		// Cleanup
		File::Destroy();
		VDAT.clear();
		VGPU.clear();
		UpdatedVDAT.clear();
		UpdatedVGPU.clear();
	}
};