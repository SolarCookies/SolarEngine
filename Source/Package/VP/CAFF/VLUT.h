#pragma once
#include <vector>

class VLUT
{
public:
	VLUT() = default;
	~VLUT() = default;

	bool Loaded;
	void Load(std::vector<unsigned char>& Full) {
		RAW = Full;
		Loaded = true;
	}

	void clear() { 
		RAW.clear(); 
	}


private:
	std::vector<unsigned char> RAW;
};