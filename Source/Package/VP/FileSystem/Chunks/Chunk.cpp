#include "Chunk.h"

Chunk::Chunk(const std::vector<unsigned char>& rawVDAT, const std::vector<unsigned char>& rawVGPU, bool& IsBig, std::string& name, ChunkInfo& Info,int cAFFIndex)
	: File(rawVDAT, IsBig), VDAT(rawVDAT), VGPU(rawVGPU), Name(name), info(Info), CAFFIndex(cAFFIndex)
{
	//set timestamp from name
	size_t firstComma = name.find(',');
	size_t secondComma = name.find(',', firstComma + 1);
	if (firstComma != std::string::npos && secondComma != std::string::npos) {
		std::string timestampStr = name.substr(firstComma + 1, secondComma - firstComma - 1);
		FileTimestamp = std::stol(timestampStr);
	}
	else {
		FileTimestamp = 0;
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
	//set NameWithoutMetadata
	NameWithoutMetadata = name.substr(0, firstComma);
};