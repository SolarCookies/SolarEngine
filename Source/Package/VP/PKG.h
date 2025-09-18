#pragma once
#include "CAFF.h"

#include "../../Windows/Log.hpp"

#include <vector>
#include <fstream>
#include <filesystem>  

struct PKGHeader
{
	uint32_t Version; // 0x00
	uint32_t CAFFCount; // 0x04
};
struct CAFFInfo
{
	uint32_t Unknown1; // 0x00
	uint32_t Offset; // 0x04
	uint32_t Size; // 0x08
};

class PKG
{
public:
	PKG() = default;
	~PKG() = default;

	void clear()
	{
		caffs.clear();
	}

	void Load(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file) {
			Log("Failed to open file: " + path, EType::Error);
			return; //File not found or could not be opened
		}
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		BYTES buffer(static_cast<size_t>(size));
		if (size > 0 && file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			PKGHeader header;
			std::memcpy(&header, buffer.data(), sizeof(PKGHeader));

			if (header.Version != 0x01) {
				Log("Unsupported PKG version: " + std::to_string(header.Version), EType::Error);
				file.close();
				return; // Unsupported version
			}

			//Retrieve CAFF information in pkg
			for(uint32_t i = 0; i < header.CAFFCount; ++i) {
				CAFFInfo info;
				std::memcpy(&info, buffer.data() + sizeof(PKGHeader) + i * sizeof(CAFFInfo), sizeof(CAFFInfo));
				caffInfos.push_back(info);
			}

			//Load CAFFs
			for (const auto& info : caffInfos) {
				if (info.Offset + info.Size > size) {
					Log("CAFF info out of bounds: Offset = " + std::to_string(info.Offset) + ", Size = " + std::to_string(info.Size), EType::Error);
					continue; // Skip invalid CAFF info
				}

				// Get CAFF Bytes
				BYTES Caff;
				Caff.resize(info.Size);
				std::memcpy(Caff.data(), buffer.data() + info.Offset, info.Size);

				// Create CAFF object and load it
				CAFF caff;
				caff.Load(Caff);
				caffs.push_back(caff);
			}
		}
		file.close();
	}
private:
	std::vector<CAFF> caffs;
	std::vector<CAFFInfo> caffInfos;
};
