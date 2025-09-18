#pragma once
#include <vector>
#include "../../../Utils/ZLibHelpers.h"

#include "Chunk.h"

class CAFF; // Forward declaration

struct VREFInfo {
	std::vector<std::string> ChunkNames;
	uint32_t VGPU_Offset = 0;
	uint32_t VGPU_Compressed_Size = 0;
	uint32_t VGPU_Uncompressed_Size = 0;
	uint32_t VDAT_Offset = 0;
	uint32_t VDAT_Compressed_Size = 0;
	uint32_t VDAT_Uncompressed_Size = 0;
	uint32_t NameBlockOffset = 0;
	uint32_t InfoBlockOffset = 0;
	std::vector<ChunkInfo> ChunkInfos;
};

class VREF
{
public:
	VREF() = default;
	~VREF() = default;

	void clear() { 
		RAW.clear(); 
	}

	bool VREFTypeCheck(BYTES& VREF) {
		bool isDataOnly = false;

		//Data would be "1635017060" in little endian because it is "data" in ASCII
		uint32_t DataType = Vince::ConvertBytesToInt(VREF, 34, false);

		if (DataType == 1635017060) {
			isDataOnly = true;
			return isDataOnly;
		}

		isDataOnly = false;
		return isDataOnly;
	}

	//Reads the VREF chunk info and fills the VREF struct with the chunk names and chunk info
	void Read_VREF_Chunks(BYTES& VREFBYTES, CAFF& caff, VREFInfo& vref, bool BigEndian);

	//Reads the VREF Header and Chunk infos
	VREFInfo Read_VREFInfo(BYTES& VREFBYTES, CAFF& caff, bool BigEndian);

	bool Loaded;
	VREFInfo vrefInfo;
	std::vector<Chunk> Chunks;

	void Load(std::vector<unsigned char>& data, CAFF& caff) {
		RAW = data;
		vrefInfo = Read_VREFInfo(data, caff, false);
		for(const auto& chunkName : vrefInfo.ChunkNames) {
			Log("Chunk Name: " + chunkName, EType::PURPLE);
		}

		Loaded = true;
	}

private:
	std::vector<unsigned char> RAW;

};