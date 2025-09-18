#pragma once
#include "CAFF/VREF.h"
#include "CAFF/VDAT.h"
#include "CAFF/VGPU.h"
#include "CAFF/VLUT.h"

#include "../../Utils/ZLibHelpers.h"

#include <string>


struct CAFFHeader {  
	char Magic[4]; // "CAFF"  
	char Version[16]; // "07.08.06.0036"  
	uint32_t HeaderSize = 120;  
	uint32_t Checksum = 0;  
	uint32_t ChunkCount = 0;  
	uint32_t ChunkSpreadCount = 0;  
	uint32_t Unknown1 = 0;  
	uint32_t Unknown2 = 0;  
	uint32_t Unknown3 = 0;  
	uint32_t Unknown4 = 0;  
	uint32_t Unknown5 = 0;  
	uint32_t Unknown6 = 0;  
	uint32_t Unknown7 = 0;  
	uint32_t Unknown8 = 0;  
	uint32_t Unknown9 = 0;  
	uint32_t Unknown10 = 0;  
	uint32_t Unknown11 = 0;  
	uint32_t VREF_Uncompressed_Size = 0;  
	uint32_t Unknown12 = 0;  
	uint32_t Unknown13 = 0;  
	uint32_t Unknown14 = 0;  
	uint32_t VREF_Compressed_Size = 0;  
	uint32_t VLUT_Uncompressed_Size = 0;  
	uint32_t Unknown15 = 0;  
	uint32_t Unknown16 = 0;  
	uint32_t Unknown17 = 0;  
	uint32_t VLUT_Compressed_Size = 0;  
};

class CAFF
{
public:
	CAFF() = default;
	~CAFF() = default;
	VREF vref;
	VLUT vlut;
	VDAT vdat;
	VGPU vgpu;

	void clear()
	{
		vref.clear();
		vdat.clear();
		vgpu.clear();
		vlut.clear();
	}

	CAFFHeader ParseHeader(const std::vector<unsigned char>& data)
	{
		CAFFHeader header;
		std::memcpy(&header, data.data(), sizeof(CAFFHeader));
		return header;
	}

	std::vector<unsigned char> ExtractZlibStream(std::vector<unsigned char>& data, size_t UncompressedSize)
	{
		if(data.size() == UncompressedSize) {
			return data; // No compression needed, return original data
		}
		else {
			return Vince::DecompressData(data, UncompressedSize); // Decompress the data
		}
		return std::vector<unsigned char>();
	}

	void Load(const std::vector<unsigned char>& data)
	{
		clear();
		header = ParseHeader(data);

		//VREF (Must come first because it contains the offsets and sizes of 2 of the 4 streams) (Stream 1)
		std::vector<unsigned char> CompressedVREFData;
		CompressedVREFData.resize(header.VREF_Compressed_Size);

		//Memcpy is only safe here because we resized the vector to the correct size, otherwise it would be unsafe.
		std::memcpy(CompressedVREFData.data(), data.data() + sizeof(CAFFHeader), header.VREF_Compressed_Size);

		std::vector<unsigned char> DecompressedVREFData = Vince::DecompressData(CompressedVREFData, header.VREF_Uncompressed_Size);
		vref.Load(DecompressedVREFData, *this);

		//VLUT (Stream 2)
		std::vector<unsigned char> CompressedVLUTData;
		CompressedVLUTData.resize(header.VLUT_Compressed_Size);

		//Memcpy is only safe here because we resized the vector to the correct size, otherwise it would be unsafe.
		std::memcpy(CompressedVLUTData.data(), data.data() + sizeof(CAFFHeader), header.VLUT_Compressed_Size);

		std::vector<unsigned char> DecompressedVLUTData = Vince::DecompressData(CompressedVLUTData, header.VLUT_Uncompressed_Size);
		vlut.Load(DecompressedVLUTData);

		//VDAT (Stream 3)
		std::vector<unsigned char> CompressedVDATData;
		CompressedVDATData.resize(vref.vrefInfo.VDAT_Compressed_Size);

		//Memcpy is only safe here because we resized the vector to the correct size, otherwise it would be unsafe.
		std::memcpy(CompressedVDATData.data(), data.data() + sizeof(CAFFHeader), vref.vrefInfo.VDAT_Compressed_Size);

		std::vector<unsigned char> DecompressedVDATData = Vince::DecompressData(CompressedVDATData, vref.vrefInfo.VDAT_Uncompressed_Size);
		vlut.Load(DecompressedVDATData);
	}

	CAFFHeader header;
private:
};
