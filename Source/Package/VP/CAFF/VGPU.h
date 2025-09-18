#pragma once
#include <vector>

class VGPU
{
public:
	VGPU() = default;
	~VGPU() = default;

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