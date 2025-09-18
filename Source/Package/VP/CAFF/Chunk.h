#pragma once
#include <string>
#include <vector>


struct ChunkInfoOffsets {
	uint32_t VDAT_Offset_Location = 0;
	uint32_t VDAT_Size_Location = 0;
	uint32_t VGPU_Offset_Location = 0;
	uint32_t VGPU_Size_Location = 0;
};

struct ChunkInfo {
	std::string ChunkName = ""; // aid_reqspinata_pinata_actor_animal_mole_variant2,1171096086,2.53
	uint32_t ID = 0;
	uint32_t VDAT_Offset = 0;
	uint32_t VDAT_Size = 0;
	BYTE VDAT_File_Data_1 = 0;
	BYTE VDAT_File_Data_2 = 0;
	bool HasVGPU = false;
	uint32_t VGPU_Offset = 0;
	uint32_t VGPU_Size = 0;
	BYTE VGPU_File_Data_1 = 0;
	BYTE VGPU_File_Data_2 = 0;
	ChunkInfoOffsets OffsetLocations;
	BYTE DebugData = 0; //Used for debugging when checking if a offset is correct
};


class Chunk {
public:
	Chunk(ChunkInfo& info) {
		chunkInfo = info;
	}
	~Chunk() = default;

	std::string Name = ""; // aid_reqspinata_pinata_actor_animal_mole_variant2
	std::string Timestamp = ""; // Unix Time (1171096086)
	float LoadTime = 0.0f; // 2.53

	ChunkInfo chunkInfo;

	void Load(std::vector<unsigned char>& FullVDAT, std::vector<unsigned char>& FullVGPU) {

	}

protected:
	std::vector<unsigned char> VDAT;
	std::vector<unsigned char> VGPU;
};