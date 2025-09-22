#pragma once
#include "../../File.h"
//Formerly known as VLUT (It was named VLUT based on a speculation that isnt confirmed yet)

class VUNK : public File {
public:
	VUNK() : File() {};
	VUNK(const std::vector<unsigned char>& rawFile) : File(rawFile) {};
	VUNK(const std::vector<unsigned char>& rawFile, bool IsBig) : File(rawFile, IsBig) {};
	void LoadFile(const std::vector<unsigned char>& rawFile) override {

	}
};