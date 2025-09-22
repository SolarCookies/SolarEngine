#pragma once
#include "../../File.h"

#include "../../../Utils/ZLibHelpers.h"

class VDAT : public File {
public:
	std::vector<unsigned char> UncompressedData;
	VDAT() : File() {};
	VDAT(const std::vector<unsigned char>& rawFile) : File(rawFile) {};
	VDAT(const std::vector<unsigned char>& rawFile, bool IsBig, int Uncompressed, int compressed) : File(rawFile, IsBig) {
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
