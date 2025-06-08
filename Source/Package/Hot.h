#pragma once  
#include <string>  
#include <vector>  
#include <cstddef>
#include <fstream>
#include <filesystem>  

#include "../Utils/ZLibHelpers.h"

struct HotFileInfo
{
	std::string Name;
	uint32_t Offset;
	uint32_t C_Size; //Compressed size
	uint32_t U_Size; //Uncompressed size
	uint32_t HeaderSize;
	uint32_t HeaderOffset;
	BYTES Data; //Decompressed data of file
};

struct HotFile
{
	std::vector<HotFileInfo> Files;
};

namespace Hot
{
	HotFile ReadFile(const std::string& filePath)
	{
		std::ifstream file(filePath, std::ios::binary | std::ios::ate);
		if (!file) {
			return HotFile(); // Return an empty HotFile on failure
		}
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		BYTES buffer(static_cast<size_t>(size));
		if (size > 0 && file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			std::cout << "File read successfully: " << filePath << std::endl;
			std::cout << "File size: " << size << " bytes" << std::endl;

			//Get how many files are in the hot file
			BYTES TempBytes = Vince::CopyBytes(buffer, 24, 4); // File count
			uint32_t FileCount = Vince::ConvertBytesToInt(TempBytes, false);

			HotFile hotFile;

			//Each file in the hot file has a header of 32 bytes starting at offset 28. At offset 16 relative to the start of the files header so "28+(i*32)" there will be 3 ints, the first is the offset of the file in the hot file, the second is the compressed size of the file, and the third is the uncompressed size of the file. The name of the file is at offset 0 relative to the start of the files header, so "28+(i*32)+4" will be the name of the file.
			for(uint32_t i = 0; i < FileCount; ++i) {
				HotFileInfo FileInfo;
				uint32_t FileHeaderOffset = 28 + (i * 32);

				TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset, 4); // U_Size
				uint32_t NameOffset = Vince::ConvertBytesToInt(TempBytes, false);

				uint32_t FileNameOffset = (28 + (FileCount * 32)) + NameOffset;

				//read null-terminated string for file name at FileNameOffset
				std::string FileName;
				for (size_t j = FileNameOffset; j < buffer.size(); ++j) {
					if (buffer[j] == 0) break; // Stop at null terminator
					FileName += static_cast<char>(buffer[j]);
				}
				FileInfo.Name = FileName; // Set the file name

				TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset + 16, 4); // U_Size
				FileInfo.U_Size = Vince::ConvertBytesToInt(TempBytes, false);

				TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset + 16 + 4, 4); // C_Size
				FileInfo.C_Size = Vince::ConvertBytesToInt(TempBytes, false);

				TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset + 16 + 8, 4); // Offset
				FileInfo.Offset = Vince::ConvertBytesToInt(TempBytes, false);

				//Header size
				TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset + 8, 4); // HeaderSize
				FileInfo.HeaderSize = Vince::ConvertBytesToInt(TempBytes, false);

				TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset + 12, 4); // HeaderOffset
				FileInfo.HeaderOffset = Vince::ConvertBytesToInt(TempBytes, false);

				//Check to see if the file needs to be decompressed
				if (FileInfo.C_Size == 0) {
					if(FileInfo.HeaderSize > 0) {
						//File is already uncompressed with header
						FileInfo.Data = Vince::CopyBytes(buffer, FileInfo.HeaderOffset, FileInfo.HeaderSize);
						FileInfo.Data.insert(FileInfo.Data.end(), buffer.begin() + FileInfo.Offset, buffer.begin() + FileInfo.Offset + FileInfo.U_Size);
					}
					else {
						//File is already uncompressed
						FileInfo.Data = Vince::CopyBytes(buffer, FileInfo.Offset, FileInfo.U_Size);
					}
				}
				//decompress the file
				else if (FileInfo.C_Size > 0 && FileInfo.U_Size > 0 && FileInfo.Offset > 0) {
					BYTES CompressedData = Vince::CopyBytes(buffer, FileInfo.Offset, FileInfo.C_Size);
					BYTES DecompressedData = Vince::DecompressData(CompressedData, FileInfo.U_Size);
					if (!DecompressedData.empty()) {
						FileInfo.Data = DecompressedData;
					}
					else {
						std::cerr << "Failed to decompress file." << std::endl;
						return HotFile(); // Return an empty HotFile on failure
					}
				}
				else {
					std::cerr << "Invalid sizes or offset or file." << std::endl;
					return HotFile(); // Return an empty HotFile on failure
				}

				hotFile.Files.push_back(FileInfo);
			}
			return hotFile; // Return the populated HotFile structure
		}
		else {
			std::cerr << "Failed to read file: " << filePath << std::endl;
			return HotFile(); // Return an empty HotFile on failure
		}
		return HotFile(); // Return an empty HotFile on failure
	}

	HotFile ReadFile(HotFileInfo a)
	{
		BYTES buffer = a.Data;
		std::cout << "File read successfully: " << a.Name << std::endl;

		//Get how many files are in the hot file
		BYTES TempBytes = Vince::CopyBytes(buffer, 24, 4); // File count
		uint32_t FileCount = Vince::ConvertBytesToInt(TempBytes, false);

		HotFile hotFile;

		//Each file in the hot file has a header of 32 bytes starting at offset 28. At offset 16 relative to the start of the files header so "28+(i*32)" there will be 3 ints, the first is the offset of the file in the hot file, the second is the compressed size of the file, and the third is the uncompressed size of the file. The name of the file is at offset 0 relative to the start of the files header, so "28+(i*32)+4" will be the name of the file.
		for (uint32_t i = 0; i < FileCount; ++i) {
			HotFileInfo FileInfo;
			uint32_t FileHeaderOffset = 28 + (i * 32);

			TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset, 4); // U_Size
			uint32_t NameOffset = Vince::ConvertBytesToInt(TempBytes, false);

			uint32_t FileNameOffset = (28 + (FileCount * 32)) + NameOffset;

			//read null-terminated string for file name at FileNameOffset
			std::string FileName;
			for (size_t j = FileNameOffset; j < buffer.size(); ++j) {
				if (buffer[j] == 0) break; // Stop at null terminator
				FileName += static_cast<char>(buffer[j]);
			}
			FileInfo.Name = FileName; // Set the file name

			TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset + 16, 4); // U_Size
			FileInfo.U_Size = Vince::ConvertBytesToInt(TempBytes, false);

			TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset + 16 + 4, 4); // C_Size
			FileInfo.C_Size = Vince::ConvertBytesToInt(TempBytes, false);

			TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset + 16 + 8, 4); // Offset
			FileInfo.Offset = Vince::ConvertBytesToInt(TempBytes, false);

			//Header size
			TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset + 8, 4); // HeaderSize
			FileInfo.HeaderSize = Vince::ConvertBytesToInt(TempBytes, false);

			TempBytes = Vince::CopyBytes(buffer, FileHeaderOffset + 12, 4); // HeaderOffset
			FileInfo.HeaderOffset = Vince::ConvertBytesToInt(TempBytes, false);

			//Check to see if the file needs to be decompressed
			if (FileInfo.C_Size == 0) {
				//File is already uncompressed
				if (FileInfo.HeaderSize > 0) {
					FileInfo.Data = Vince::CopyBytes(buffer, FileInfo.HeaderOffset, FileInfo.HeaderSize);
					FileInfo.Data.insert(FileInfo.Data.end(), buffer.begin() + FileInfo.Offset, buffer.begin() + FileInfo.Offset + FileInfo.U_Size);
				}
				else {
					FileInfo.Data = Vince::CopyBytes(buffer, FileInfo.Offset, FileInfo.U_Size);
				}
			}
			//decompress the file
			else if (FileInfo.C_Size > 0 && FileInfo.U_Size > 0 && FileInfo.Offset > 0) {
				BYTES CompressedData = Vince::CopyBytes(buffer, FileInfo.Offset, FileInfo.C_Size);
				BYTES DecompressedData = Vince::DecompressData(CompressedData, FileInfo.U_Size);
				if (!DecompressedData.empty()) {
					if (FileInfo.HeaderSize > 0) {
						FileInfo.Data = Vince::CopyBytes(buffer, FileInfo.HeaderOffset, FileInfo.HeaderSize);
						FileInfo.Data.insert(FileInfo.Data.end(), DecompressedData.begin(), DecompressedData.end());
					}
					else {
						FileInfo.Data = DecompressedData;
					}
				}
				else {
					std::cerr << "Failed to decompress file." << std::endl;
					return HotFile(); // Return an empty HotFile on failure
				}
			}
			else {
				std::cerr << "Invalid sizes or offset or file." << std::endl;
				return HotFile(); // Return an empty HotFile on failure
			}

			hotFile.Files.push_back(FileInfo);
		}
		return hotFile; // Return the populated HotFile structure
	}
}