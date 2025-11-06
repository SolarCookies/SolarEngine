#pragma once
#include "../File.h"

#include "../CAFF/CAFF.h"

class DebugPack : public File {
public:

	std::string Path;
	std::vector<CAFF> CAFFs;
	bool IsValid = false;
	int caffNumber = 0;

	DebugPack() = default;
	DebugPack(const std::vector<unsigned char>& rawFile, std::string& path) : File(rawFile), Path(path) {
		LoadFile(rawFile);
	};

	void LoadFile(const std::vector<unsigned char>& rawFile) override;

	void Destroy() override;
};