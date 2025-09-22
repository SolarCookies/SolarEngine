#pragma once
#include <memory>
#include <vector>
#include "Chunk.h"

#include "Images/DDS/DDS_Chunk.h"
#include "Images/RAW/RGBA_Chunk.h"
#include "Models/Model_Chunk.h"

struct ChunkInfo;


	inline bool IsFloatEqual(float a, float b, float epsilon = 0.001f) {
		return fabs(a - b) < epsilon;
	}

	static std::unique_ptr<Chunk> CreateChunk
	(
		const std::vector<unsigned char>& rawVDAT,
		const std::vector<unsigned char>& rawVGPU,
		bool& IsBig,
		std::string& name,
		ChunkInfo& Info
	) {

		//std::cout << "Creating chunk: " << name << std::endl;
		float FileType = 0.0f;
		std::string NameWithoutMetadata;

		size_t firstComma = name.find(',');
		size_t secondComma = name.find(',', firstComma + 1);
		if (firstComma != std::string::npos && secondComma != std::string::npos) {
			//TimeStamp is between the two commas
		}
		//set FileType from name
		if (secondComma != std::string::npos) {
			std::string fileTypeStr = name.substr(secondComma + 1);
			try {
				FileType = std::stof(fileTypeStr);
			}
			catch (...) {
				FileType = 0.0f;
			}
		}
		else {
			FileType = 0.0f;
		}
		NameWithoutMetadata = name.substr(0, firstComma);

		// switch using a float by casting to int
		// Also using a goto to avoid nested switches or ifs so that the file can be easily extended in the future, 
		// just make sure to call goto loc_chunk; after checking for a specific file type or it might run a bunch of checks unnecessarily

		if (IsFloatEqual(FileType, 2.42)) goto loc_image;
		if (IsFloatEqual(FileType, 4.00)) goto loc_model;
		goto loc_chunk; //default case

	loc_image:
		{
			uint16_t SizeX = 0;
			uint16_t SizeY = 0;
			int Encoding = 0;
			memcpy(&SizeX, &rawVDAT.data()[0] + 8, sizeof(uint16_t));
			memcpy(&SizeY, &rawVDAT.data()[0] + 10, sizeof(uint16_t));
			memcpy(&Encoding, &rawVDAT.data()[0], sizeof(uint32_t));

			//if first 4 bites are " dds" then file is a dds_chunk
			if (rawVGPU.size() >= 4 && rawVGPU[1] == 'd' && rawVGPU[2] == 'd' && rawVGPU[3] == 's') {
				return std::make_unique<DDS_Chunk>(rawVDAT, rawVGPU, IsBig, name, Info);
			}
			if (Encoding == 1 || Encoding == 2 || Encoding == 3) { //dds
				return std::make_unique<DDS_Chunk>(rawVDAT, rawVGPU, IsBig, name, Info);
			}
			if (Encoding >= 4) {
				return std::make_unique<RGBA_Chunk>(rawVDAT, rawVGPU, IsBig, name, Info);
			}
			goto loc_chunk;
		}

	loc_model:
		{
			if (rawVGPU.size() < 4) goto loc_chunk;
			return std::make_unique<Model_Chunk>(rawVDAT, rawVGPU, IsBig, name, Info);
		}

	loc_chunk:
		return std::make_unique<Chunk>(rawVDAT, rawVGPU, IsBig, name, Info);
	}