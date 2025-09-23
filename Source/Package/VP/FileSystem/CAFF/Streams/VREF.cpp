#include "VREF.h"
#include "../CAFF.h"

#include "../../Chunks/ChunkFactory.h"


void VREF::LoadFile(const std::vector<unsigned char>& rawFile)
{
	CheckType();
	memcpy(&VDAT_Data, &RawFile[0], sizeof(VREFSubStreamData));
	if (!Type) memcpy(&VGPU_Data, &RawFile[33], sizeof(VREFSubStreamData));

	int NameInfoBlockOffset = 0;
	int NameBlockOffset = 0;
	int infoBlockOffset = 0;
	if (!Type) memcpy(&infoBlockOffset, &RawFile[77], 4);
	else memcpy(&infoBlockOffset, &RawFile[39], 4);

	if (IsBigEndianFile) {
		VDAT_Data.Uncompressed_Size = _byteswap_ulong(VDAT_Data.Uncompressed_Size);
		VDAT_Data.Compressed_Size = _byteswap_ulong(VDAT_Data.Compressed_Size);
		NameInfoBlockOffset = 43;
		if (!Type) {
			VGPU_Data.Uncompressed_Size = _byteswap_ulong(VGPU_Data.Uncompressed_Size);
			VGPU_Data.Compressed_Size = _byteswap_ulong(VGPU_Data.Compressed_Size);
			NameInfoBlockOffset = 81;
		}
	}
	else {
		NameInfoBlockOffset = 43;
		if (!Type) {
			NameInfoBlockOffset = 81;
		}
	}
	NameBlockOffset = NameInfoBlockOffset + (chunkCount * 4);

	std::vector<int> NameOffsets;
	NameOffsets.resize(chunkCount);
	for (int i = 0; i < chunkCount; i++) {
		int offset;
		memcpy(&offset, &RawFile[NameInfoBlockOffset + (i * 4)], 4);
		if (IsBigEndianFile) offset = _byteswap_ulong(offset);
		NameOffsets[i] = offset + NameBlockOffset;
	}


	Names.resize(chunkCount);
	for (int i = 0; i < chunkCount; i++) {
		std::string name;
		for (int j = NameOffsets[i]; j < RawFile.size(); j++) {
			if (RawFile[j] == 0) break;
			name += RawFile[j];
		}
		Names[i] = name;
	}

	for (int i = 0; i < chunkCount; i++) {
		int offset;
		memcpy(&offset, &RawFile[NameInfoBlockOffset + (i * 4)], 4);
		if (IsBigEndianFile) offset = _byteswap_ulong(offset);
		NameOffsets[i] = offset + NameBlockOffset;
	}

	ChunkInfos.resize(chunkCount);
	int offset = infoBlockOffset + NameBlockOffset + 4;

	for (int i = 0; i < chunkCount; i++) {
		ChunkInfo VDatInfo;
		memcpy(&VDatInfo.VDat, &RawFile[offset], sizeof(VDatInfo.VDat));
		VDatInfo.Offset1 = offset;
		offset += sizeof(VDatInfo.VDat);
		int nextID = 0;
		memcpy(&nextID, &RawFile[offset], 4);
		if (nextID == VDatInfo.VDat.ID) {
			VDatInfo.Offset2 = offset;
			memcpy(&VDatInfo.VGpu, &RawFile[offset], sizeof(VDatInfo.VGpu));
			VDatInfo.HasGpu = true;
			offset += sizeof(VDatInfo.VGpu);
		}
		else {
			VDatInfo.VGpu.ID = 0;
			VDatInfo.VGpu.Offset = 0;
			VDatInfo.VGpu.Size = 0;
			VDatInfo.HasGpu = false;
		}

		if (IsBigEndianFile) {
			VDatInfo.VDat.ID = _byteswap_ulong(VDatInfo.VDat.ID);
			VDatInfo.VDat.Offset = _byteswap_ulong(VDatInfo.VDat.Offset);
			VDatInfo.VDat.Size = _byteswap_ulong(VDatInfo.VDat.Size);
			if (VDatInfo.HasGpu) {
				VDatInfo.VGpu.ID = _byteswap_ulong(VDatInfo.VGpu.ID);
				VDatInfo.VGpu.Offset = _byteswap_ulong(VDatInfo.VGpu.Offset);
				VDatInfo.VGpu.Size = _byteswap_ulong(VDatInfo.VGpu.Size);
			}
		}

		ChunkInfos[i] = VDatInfo;


	}


}

void VREF::LoadChunks(CAFF& caff)
{
	int chunkIndex = 0;
	for(ChunkInfo& chunkI : ChunkInfos)
	{

		std::vector<unsigned char> chunkVDAT;
		chunkVDAT.resize(chunkI.VDat.Size);
		memcpy(chunkVDAT.data(), caff.vdat->UncompressedData.data() + chunkI.VDat.Offset, chunkI.VDat.Size);
		std::vector<unsigned char> chunkVGPU;
		if (chunkI.VGpu.Size > 0 && Type == 0) {
			chunkVGPU.resize(chunkI.VGpu.Size);
			memcpy(chunkVGPU.data(), caff.vgpu->UncompressedData.data() + chunkI.VGpu.Offset, chunkI.VGpu.Size);
		}
			
		if (chunkI.VDat.ID > 0 && chunkI.VDat.ID <= Names.size()) {
			std::string& name = Names[chunkI.VDat.ID - 1];
			std::unique_ptr <Chunk> C = CreateChunk(chunkVDAT, chunkVGPU, IsBigEndianFile, Names[chunkI.VDat.ID - 1], chunkI, CAFFIndex);
			caff.chunks.push_back(std::move(C));
		}
		else {
			std::cout << "Warning: VREF chunk with invalid ID: " << chunkI.VDat.ID << std::endl;
		}
		chunkIndex++;
	}
}
