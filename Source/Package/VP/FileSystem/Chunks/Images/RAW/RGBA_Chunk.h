#pragma once
#include "../../Chunk.h"
#include "../../../GlobalPackages.h"



class RGBA_Chunk : public Chunk
{
public:
    std::vector<unsigned char> ARGBImage;
	std::vector<unsigned char> RGBAImage;
    //image handle for thumbnail rendering
    Texture RGBATexture;
    uint16_t SizeX = 0;
    uint16_t SizeY = 0;
	bool loaded = false;
	uint32_t Encoding = 0;

	unsigned int TextureSwizzle_40AB60(unsigned int a1, unsigned int a2, unsigned int a3, int a4)
	{
		int v4;
		int v5;
		unsigned int v6;

		v4 = (a2 >> 2) + (a2 >> 1 >> (a2 >> 2));
		v5 = ((a3 & 7) + 4 * (a1 & 6)) << ((a2 >> 2) + (a2 >> 1 >> (a2 >> 2)));
		v6 = (v5 & 0xF)
			+ ((a1 & 8) << (v4 + 3))
			+ (((a3 >> 5) + (a1 >> 5) * ((unsigned int)(a4 + 31) >> 5)) << (v4 + 7))
			+ 2 * ((v5 & 0xFFFFFFF0) + 8 * (a1 & 1));
		return ((v6 & 0x3F) + ((8 * ((unsigned char)a3 + 2 * (a1 & 0xF8))) & 0xC0) + 4 * ((v6 & 0x1C0) + 2 * ((v6 & 0xFFFFFE00) + 16 * (a1 & 0x10)))) >> v4;
	}

    RGBA_Chunk() : Chunk() {};
    RGBA_Chunk(const std::vector<unsigned char>& rawFile) : Chunk(rawFile) {};
    RGBA_Chunk(const std::vector<unsigned char>& rawVDAT, const std::vector<unsigned char>& rawVGPU, bool& IsBig, std::string& name, ChunkInfo& Info, int cAFFIndex)
        : Chunk(rawVDAT, rawVGPU, IsBig, name, Info, cAFFIndex)
    {
		
        memcpy(&SizeX, &rawVDAT.data()[0] + 8, sizeof(uint16_t));
        memcpy(&SizeY, &rawVDAT.data()[0] + 10, sizeof(uint16_t));
		memcpy(&Encoding, &rawVDAT.data()[0], sizeof(uint32_t));

		if (IsBig) {
			SizeX = _byteswap_ushort(SizeX);
			SizeY = _byteswap_ushort(SizeY);
			Encoding = _byteswap_ulong(Encoding);
		}

		RGBAImage.resize(SizeX * SizeY * 4);
		ARGBImage = rawVGPU;
		RGBAImage.resize(ARGBImage.size());

		if (Encoding == 4) {//Texture Type 21
			for (int y = 0; y < SizeY; y++) {
				for (int x = 0; x < SizeX; x++) {
					unsigned idx = TextureSwizzle_40AB60(y, 4u, x, SizeX);
					const unsigned char* src = &ARGBImage[idx * 4];
					unsigned char* dst = &RGBAImage[(y * SizeX + x) * 4];

					dst[0] = src[1]; // R
					dst[1] = src[2]; // G
					dst[2] = src[3]; // B
					dst[3] = src[0]; // A
				}
			}
			GLenum pixelType = GL_UNSIGNED_BYTE;
			int w = SizeX;
			int h = SizeY;
			if (!ExtractAll) {
				RGBATexture = Texture(RGBAImage, pixelType, w, h);
				AddTexture(CAFFIndex, Info.VDat.ID, &RGBATexture);
				loaded = true;
			}
		}
		else {

			for (int i = 0; i < ARGBImage.size() / 4; i++)
			{
				RGBAImage[i * 4 + 0] = ARGBImage[i * 4 + 1]; //R
				RGBAImage[i * 4 + 1] = ARGBImage[i * 4 + 2]; //G
				RGBAImage[i * 4 + 2] = ARGBImage[i * 4 + 3]; //B
				RGBAImage[i * 4 + 3] = ARGBImage[i * 4 + 0]; //A
			}
			GLenum pixelType = GL_UNSIGNED_BYTE;
			int w = SizeX;
			int h = SizeY;
			if (!ExtractAll) {
				RGBATexture = Texture(RGBAImage, pixelType, w, h);
				AddTexture(CAFFIndex, Info.VDat.ID, &RGBATexture);
				loaded = true;
			}
		}

		if (NameWithoutMetadata.find("madeup") != std::string::npos) {
			//generate a name, make a hash of the data or something and append it to aid_texture_madeup
			std::string ddsHash = HashData(RGBAImage);
			NameWithoutMetadata = "aid_texture_madeup_" + ddsHash.substr(0, 8); // Use first 8 characters of the hash for uniqueness
		}
    };

    void RenderListItem() override {
        if (loaded) {
			
			ImGui::Image((ImTextureID)(intptr_t)RGBATexture.ID, ImVec2(50, 50));
        }
        ImGui::SameLine();
        Chunk::RenderListItem();

    };

    void RenderListClick() override {
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
					PendingChange = true;
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
						PendingChange = true;
					}
				}
			}

			if(ImGui::Button("View Texture"))
			{
				PreviewTexture = &RGBATexture;
			}
			if (ImGui::Button("Export Texture"))
			{
				ExportInfo ex = ExportChunk();
				
				//Get desktop path
				std::string DumpPath;
				char* userProfile = nullptr;
				size_t len = 0;
				_dupenv_s(&userProfile, &len, "USERPROFILE");
				if (userProfile) {
					DumpPath = std::string(userProfile) + "\\Desktop\\" + NameWithoutMetadata + ".png";
					free(userProfile);
				}
				else {
					DumpPath = NameWithoutMetadata + ".png";
				}

				ex.SaveFunction(DumpPath);
			}
		}
    };

    void RenderListHover() override {
        Chunk::RenderListHover();
        

    };

	ExportInfo ExportChunk() override;

	void Destroy() override {
		// Cleanup
		Chunk::Destroy();
		if (PreviewTexture == &RGBATexture) PreviewTexture = nullptr;
		RGBAImage.clear();
		ARGBImage.clear();
		RGBATexture.Delete();
	}
};