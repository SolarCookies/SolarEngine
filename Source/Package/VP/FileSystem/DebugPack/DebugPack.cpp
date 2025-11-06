#include "DebugPack.h"


void DebugPack::LoadFile(const std::vector<unsigned char>& rawFile)
{
	std::cout << "Loading Debug Pack: " << Path << std::endl;
	//check if file exists
	if (!std::filesystem::exists(Path)) {
		std::cout << "File does not exist: " << Path << std::endl;
		return;
	}
	else {

		std::cout << "File exists: " << Path << std::endl;
		CAFFs.clear();
		std::vector<std::vector<unsigned char>> RawCAFFS;

		// Read the file into a buffer
		std::ifstream file(Path, std::ios::binary | std::ios::ate);
		if (!file) {
			std::cout << "Failed to open file: " << Path << std::endl;
			return;
		}
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint8_t> buffer(size);
		if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			std::cout << "Failed to read file: " << Path << std::endl;
			return;
		}

		// Search for "CAFF" markers
		const std::vector<uint8_t> marker = { 0x43, 0x41, 0x46, 0x46 }; // "CAFF"
		size_t pos = 0;
		std::vector<size_t> marker_positions;

		// Find all marker positions
		while (pos + marker.size() <= buffer.size()) {
			if (std::equal(marker.begin(), marker.end(), buffer.begin() + pos)) {
				marker_positions.push_back(pos);
			}
			++pos;
		}

		// Extract segments between markers
		for (size_t i = 0; i < marker_positions.size(); ++i) {
			size_t start = marker_positions[i];
			size_t end = (i + 1 < marker_positions.size()) ? marker_positions[i + 1] : buffer.size();
			std::vector<unsigned char> segment(buffer.begin() + start, buffer.begin() + end);
			RawCAFFS.push_back(segment);
		}
		caffNumber = RawCAFFS.size() - 1;
		std::cout << "Found " << RawCAFFS.size() << " CAFF segments." << std::endl;
		for (int i = 0; i < RawCAFFS.size(); i++) {
			CAFF caff = CAFF(RawCAFFS[i], true, i - 1, true);
			CAFFs.push_back(std::move(caff));
		}
		IsValid = true;
	}

}

void DebugPack::Destroy()
{
	File::Destroy();
	for (CAFF& caff : CAFFs) {
		caff.Destroy();
	}
	CAFFs.clear();
}
