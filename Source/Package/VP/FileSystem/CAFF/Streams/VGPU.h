#pragma once
#include "../../File.h"

#include "../../../Utils/ZLibHelpers.h"

class VGPU : public File {
public:
	std::vector<unsigned char> UncompressedData;
	VGPU() : File() {};
	VGPU(const std::vector<unsigned char>& rawFile) : File(rawFile) {};
	VGPU(const std::vector<unsigned char>& rawFile, bool IsBig, int Uncompressed, int compressed) : File(rawFile, IsBig) {
		if (compressed > 0) {
			std::vector<unsigned char> rawFileCopy = rawFile;
			UncompressedData = Vince::DecompressData(rawFileCopy, Uncompressed);
		}
		else {
			UncompressedData = rawFile;
		}
	};
	void LoadFile(const std::vector<unsigned char>& rawFile) override {

	}
	void Destroy() override {
		File::Destroy();
		UncompressedData.clear();

	}
};
