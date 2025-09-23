#pragma once
#include "../../File.h"
#include "VDAT.h"
#include "VGPU.h"

#include "../../Chunks/Chunk.h"

class CAFF;


#pragma pack(push, 1)
struct VREFSubStreamData {
	char Unk1[9];
	int Uncompressed_Size;
	char Unk2[16];
	int Compressed_Size;
};
#pragma pack(pop)

class VREF : public File {
public:
	VREFSubStreamData VDAT_Data;
	VREFSubStreamData VGPU_Data;
	int chunkCount = 0;
	int CAFFIndex = 0;
	std::vector<ChunkInfo> ChunkInfos;
	std::vector<std::string> Names;
	CAFF& caffRef = *(CAFF*)nullptr;

	//Used to determine if the CAFF has a gpu file
	bool Type = false;
	void CheckType() {
		int type;
		memcpy(&type, &RawFile[34], 4);
		if(type == 1635017060 || type == 1684108385) Type = true;
		else Type = false;
	}

	VREF() : File() {};
	VREF(const std::vector<unsigned char>& rawFile) : File(rawFile) {};

	VREF(const std::vector<unsigned char>& rawFile, bool IsBig, int ChunkCount, int Index) : File(rawFile, IsBig), chunkCount(ChunkCount), CAFFIndex(Index){
		LoadFile(rawFile);
	};
	void LoadFile(const std::vector<unsigned char>& rawFile) override;

	void LoadChunks(CAFF& caff);

	void Destroy() override {
		File::Destroy();
		ChunkInfos.clear();
		Names.clear();
		Type = false;
		chunkCount = 0;

	}
};