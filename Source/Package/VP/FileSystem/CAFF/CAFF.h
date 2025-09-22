#include "Streams/Streams.h"
#include "../File.h"
#include "../../Utils/ZLibHelpers.h"
#include "../../../../GlobalSettings.h"
#include "../../Utils/OpenFIleDialog.h"
#include "../Chunks/Chunk.h"


struct CAFFHeader {
	char Magic[4] = {};
    char Version[16] = {};
    int VREF_Offset = 0; //20
    int Checksum = 0;
    int ChunkCount = 0; //28
    int ChunkSpreadCount = 0; //32
    char Unk2[44] = {};
    int VREF_Uncompressed_Size = 0; //80
    char Unk3[12] = {};
    int VREF_Compressed_Size = 0; //96
    int VUNK_Uncompressed_Size = 0; //100
    char Unk4[12] = {};
    int VUNK_Compressed_Size = 0; //116
};

class CAFF : public File {
protected:
	bool PendingChanges = false;
public:

	CAFFHeader header;
	std::vector<std::unique_ptr<Chunk>> chunks;
	std::unique_ptr <VREF> vref;
	std::unique_ptr <VDAT> vdat;
	std::unique_ptr <VGPU> vgpu;
	std::unique_ptr <VUNK> vunk;

	// Adding a constructor to accept rawFile data
	CAFF() : File() {};
	CAFF(const std::vector<unsigned char>& rawFile) : File(rawFile) {};
    CAFF(const std::vector<unsigned char>& rawFile, bool IsBig) : File(rawFile,IsBig) {
		LoadFile(rawFile);
	};

	void LoadFile(const std::vector<unsigned char>& rawFile) override {
		memcpy(&header, rawFile.data(), sizeof(CAFFHeader));
        if(IsBigEndianFile) {
			header.VREF_Offset = _byteswap_ulong(header.VREF_Offset);
			header.ChunkCount = _byteswap_ulong(header.ChunkCount);
			header.ChunkSpreadCount = _byteswap_ulong(header.ChunkSpreadCount);
			header.VREF_Uncompressed_Size = _byteswap_ulong(header.VREF_Uncompressed_Size);
			header.VREF_Compressed_Size = _byteswap_ulong(header.VREF_Compressed_Size);
			header.VUNK_Uncompressed_Size = _byteswap_ulong(header.VUNK_Uncompressed_Size);
			header.VUNK_Compressed_Size = _byteswap_ulong(header.VUNK_Compressed_Size);
		}

		//Load VREF
		bool vrefCompressed = (header.VREF_Compressed_Size != 0 && header.VREF_Compressed_Size != header.VREF_Uncompressed_Size);
		std::vector<unsigned char> VREFData;

		//Some CAFFs are uncompressed, when this is the case the Compressed size is = to the uncompressed size
		if (!vrefCompressed) {
			VREFData.resize(header.VREF_Uncompressed_Size);
			memcpy(VREFData.data(), rawFile.data() + header.VREF_Offset, header.VREF_Uncompressed_Size);
		}
		else {
			//The caff seems to be compressed, so we need to decompress it using zlib
			std::vector<unsigned char> CompressedVREF(header.VREF_Compressed_Size);
			memcpy(CompressedVREF.data(), rawFile.data() + header.VREF_Offset, header.VREF_Compressed_Size);
			VREFData = Vince::DecompressData(CompressedVREF, header.VREF_Uncompressed_Size);
		}

		vref = std::make_unique<VREF>(VREFData, IsBigEndianFile, header.ChunkCount);

		//Load VUNK
		std::vector<unsigned char> VUNKData;
		int VUNKOffset = 0;
		int VUNKSize = 0;
		if (vrefCompressed) {
			VUNKOffset = header.VREF_Offset + header.VREF_Compressed_Size;
		}
		else {
			VUNKOffset = header.VREF_Offset + header.VREF_Uncompressed_Size;
		}

		if (header.VUNK_Compressed_Size == 0 || header.VUNK_Compressed_Size == header.VUNK_Uncompressed_Size) {
			VUNKData.resize(header.VUNK_Uncompressed_Size);
			VUNKSize = header.VUNK_Uncompressed_Size;
			memcpy(VUNKData.data(), rawFile.data() + header.VREF_Offset + header.VREF_Compressed_Size, header.VUNK_Uncompressed_Size);
		}
		else {
			std::vector<unsigned char> CompressedVUNK(header.VUNK_Compressed_Size);
			memcpy(CompressedVUNK.data(), rawFile.data() + header.VREF_Offset + header.VREF_Compressed_Size, header.VUNK_Compressed_Size);
			VUNKSize = header.VUNK_Compressed_Size;
			VUNKData = Vince::DecompressData(CompressedVUNK, header.VUNK_Uncompressed_Size);
		}

		vunk = std::make_unique<VUNK>(VUNKData, IsBigEndianFile);

		//Load VDAT and VGPU using offsets/sizes stored in the VREF file
		std::vector<unsigned char> VDATData;
		if (vref->VDAT_Data.Compressed_Size != 0) {
			VDATData.resize(vref->VDAT_Data.Compressed_Size);
			memcpy(VDATData.data(), rawFile.data() + VUNKOffset + VUNKSize, vref->VDAT_Data.Compressed_Size);
			vdat = std::make_unique<VDAT>(VDATData, IsBigEndianFile, vref->VDAT_Data.Uncompressed_Size, vref->VDAT_Data.Compressed_Size);
		}
		else {
			VDATData.resize(vref->VDAT_Data.Uncompressed_Size);
			memcpy(VDATData.data(), rawFile.data() + VUNKOffset + VUNKSize, vref->VDAT_Data.Uncompressed_Size);
			vdat = std::make_unique<VDAT>(VDATData, IsBigEndianFile, vref->VDAT_Data.Uncompressed_Size, vref->VDAT_Data.Compressed_Size);
		}

		if (vref->Type) { 
			//make empty vgpu to avoid crashing on some CAFFs that don't have VGPU data but have a non zero VGPU size in the VREF
			vgpu = std::make_unique<VGPU>();
			vref->LoadChunks(*this);
			return;
		}

		std::vector<unsigned char> VGPUData;
		if (vref->VGPU_Data.Compressed_Size != 0) {
			VGPUData.resize(vref->VGPU_Data.Compressed_Size);
			memcpy(VGPUData.data(), rawFile.data() + VUNKOffset + VUNKSize + VDATData.size(), vref->VGPU_Data.Compressed_Size);
			vgpu = std::make_unique<VGPU>(VGPUData, IsBigEndianFile, vref->VGPU_Data.Uncompressed_Size, vref->VGPU_Data.Compressed_Size);
		}
		else {
			VGPUData.resize(vref->VGPU_Data.Uncompressed_Size);
			memcpy(VGPUData.data(), rawFile.data() + VUNKOffset + VUNKSize + VDATData.size(), vref->VGPU_Data.Uncompressed_Size);
			vgpu = std::make_unique<VGPU>(VGPUData, IsBigEndianFile, vref->VGPU_Data.Uncompressed_Size, vref->VGPU_Data.Compressed_Size);
		}


		vref->LoadChunks(*this);
		
	}

	bool HasPendingChanges(){
		if (PendingChanges) return true;
		for(const auto& chunk : chunks) {
			if (chunk->PendingUpdate) {
				PendingChanges = true;
				return true;
			}
		}
		PendingChanges = false;
		return false;
	}

	void Destroy() override {
		for(const auto& chunk : chunks) {
			chunk->Destroy();
		}
		chunks.clear();
		if (vref) vref->Destroy();
		if (vdat) vdat->Destroy();
		if (vgpu) vgpu->Destroy();
		if (vunk) vunk->Destroy();
	}

	void RenderListClick(std::string& CurrentCAFF, int& i) {

		if (ImGui::Button("Open")) {
			CurrentCAFF = "CAFF " + std::to_string(i + 1);
		}
		if (ImGui::Button("View CAFF")) {
			FileName = "CAFF_" + std::to_string(i) + ".caff";
			FileViewType = 2;
			FileViewBuffer = RawFile;
			GlobalSaveFunction = [this](const std::vector<unsigned char>& data) {};
		}
		ImGui::SameLine();
		if (ImGui::Button("Export CAFF")) {
			bool saved = Walnut::OpenFileDialog::AskSaveFile(std::vector<unsigned char>(RawFile), "CAFF" + std::to_string(i + 1) + ".caff");
		}
		if (ImGui::Button("View VREF")) {
			FileName = "CAFF_" + std::to_string(i) + ".vref";
			FileViewType = 2;
			FileViewBuffer = vref->RawFile;
			GlobalSaveFunction = [this](const std::vector<unsigned char>& data) {};
		}
		ImGui::SameLine();
		if (ImGui::Button("View VUNK")) {
			FileName = "CAFF_" + std::to_string(i) + ".vunk";
			FileViewType = 2;
			FileViewBuffer = vunk->RawFile;
			GlobalSaveFunction = [this](const std::vector<unsigned char>& data) {};
		}
		ImGui::SameLine();
		if (ImGui::Button("View VDAT")) {
			FileName = "CAFF_" + std::to_string(i) + ".vdat";
			FileViewType = 0;
			FileViewBuffer = vdat->UncompressedData;
			GlobalSaveFunction = [this](const std::vector<unsigned char>& data) {};
		}
		ImGui::SameLine();
		if (ImGui::Button("View VGPU"))
		{
			FileName = "CAFF_" + std::to_string(i) + ".vgpu";
			FileViewType = 1;
			FileViewBuffer = vgpu->UncompressedData;
			GlobalSaveFunction = [this](const std::vector<unsigned char>& data) {};
		}

		if (ImGui::Button("Export VREF")) {
			bool saved = Walnut::OpenFileDialog::AskSaveFile(std::vector<unsigned char>(vref->RawFile), "CAFF" + std::to_string(i + 1) + ".vref");
		}
		ImGui::SameLine();
		if (ImGui::Button("Export VUNK")) {
			bool saved = Walnut::OpenFileDialog::AskSaveFile(std::vector<unsigned char>(vunk->RawFile), "CAFF" + std::to_string(i + 1) + ".vunk");
		}
		ImGui::SameLine();
		if (ImGui::Button("Export VDAT")) {
			bool saved = Walnut::OpenFileDialog::AskSaveFile(std::vector<unsigned char>(vdat->UncompressedData), "CAFF" + std::to_string(i + 1) + ".vdat");
		}
		ImGui::SameLine();
		if (ImGui::Button("Export VGPU"))
		{
			bool saved = Walnut::OpenFileDialog::AskSaveFile(std::vector<unsigned char>(vgpu->UncompressedData), "CAFF" + std::to_string(i + 1) + ".vgpu");
		}
	}
};