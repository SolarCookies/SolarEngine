#include "VREF.h"
#include "../CAFF.h"

void VREF::Read_VREF_Chunks(BYTES& VREFBYTES, CAFF& caff, VREFInfo& vref, bool BigEndian)
{

	uint32_t offset;

	if (VREFTypeCheck(VREFBYTES)) {
		offset = 43; //43 is the start of the chunk name offset (Not the name itself but the offset to the name)
	}
	else {
		offset = 81; //81 is the start of the chunk name offset (Not the name itself but the offset to the name)
	}

	//Forward declare the chunk name offset and size so we are not allocating memory in the loop
	uint32_t ChunkNameoffset;
	uint32_t NextChunkNameoffset;
	uint32_t NameSize;
	BYTES ChunkName;

	//for each chunk we get the name by getting the offset to the name and then getting the name from the name block
	for (int i = 0; i <= (caff.header.ChunkCount - 1); i++) {

		//If this is the last chunk, we need to use the NameBlock size rather then the next chunk name offset
		if (i == (caff.header.ChunkCount - 1)) {
			ChunkNameoffset = Vince::ConvertBytesToInt(VREFBYTES, offset, BigEndian);

			//NameSize = the size of the name block - the size of the chunk name offset
			NameSize = vref.InfoBlockOffset - (vref.NameBlockOffset + ChunkNameoffset);

			//ChunkName = Walnut::OpenFileDialog::CopyBytes(VREFBYTES, vref.NameBlockOffset + ChunkNameoffset, NameSize);
			//use memcpy to set chunk name to the bytes from the VREFBYTES
			ChunkName.resize(NameSize);
			memcpy(ChunkName.data(), VREFBYTES.data() + vref.NameBlockOffset + ChunkNameoffset, NameSize);

			offset += 4;

			vref.ChunkNames.push_back(Vince::ConvertBytesToString(ChunkName));

			continue;
		}

		ChunkNameoffset = Vince::ConvertBytesToInt(VREFBYTES, offset, BigEndian);

		NextChunkNameoffset = Vince::ConvertBytesToInt(VREFBYTES, offset + 4, BigEndian);

		NameSize = NextChunkNameoffset - ChunkNameoffset;

		//ChunkName = Walnut::OpenFileDialog::CopyBytes(VREFBYTES, vref.NameBlockOffset + ChunkNameoffset, NameSize);

		//use memcpy to set chunk name to the bytes from the VREFBYTES
		ChunkName.resize(NameSize);
		memcpy(ChunkName.data(), VREFBYTES.data() + vref.NameBlockOffset + ChunkNameoffset, NameSize);

		offset += 4;

		vref.ChunkNames.push_back(Vince::ConvertBytesToString(ChunkName));
	}

	//Info Block starts after a buffer of 4 bytes in-between the name block and the info block
	offset = vref.InfoBlockOffset + 4;

	//For each chunk we get the chunk info from the info block
	for (int i = 0; i <= (caff.header.ChunkCount - 1); i++) {
		ChunkInfo chunkinfo;
		ChunkInfoOffsets chunkinfooffsets;
		chunkinfo.ChunkName = vref.ChunkNames[i];
		//chunk id (4 bytes)
		chunkinfo.ID = Vince::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
		offset += 4;
		uint32_t VDATOFFSET = offset;
		chunkinfooffsets.VDAT_Offset_Location = offset;
		chunkinfo.VDAT_Offset = Vince::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
		offset += 4;
		uint32_t VDATSIZE = offset;
		chunkinfooffsets.VDAT_Size_Location = offset;
		chunkinfo.VDAT_Size = Vince::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
		offset += 4;
		chunkinfo.VDAT_File_Data_1 = VREFBYTES[offset];
		offset += 1;
		chunkinfo.VDAT_File_Data_2 = VREFBYTES[offset];
		offset += 1;
		if (VREFBYTES.size() < (offset + 4)) {
			chunkinfo.OffsetLocations = chunkinfooffsets;
			vref.ChunkInfos.push_back(chunkinfo);
		}
		else {
			uint32_t NextID = Vince::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
			if (NextID == chunkinfo.ID) {
				chunkinfo.HasVGPU = true;
				offset += 4;
				uint32_t VGPUOFFSET = offset;
				chunkinfooffsets.VGPU_Offset_Location = offset;
				chunkinfo.VGPU_Offset = Vince::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
				offset += 4;
				uint32_t VGPUSIZE = offset;
				chunkinfooffsets.VGPU_Size_Location = offset;
				chunkinfo.VGPU_Size = Vince::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
				offset += 4;
				chunkinfo.VGPU_File_Data_1 = VREFBYTES[offset];
				offset += 1;
				chunkinfo.VGPU_File_Data_2 = VREFBYTES[offset];
				offset += 1;
				chunkinfo.OffsetLocations = chunkinfooffsets;
				vref.ChunkInfos.push_back(chunkinfo);
			}
			else {
				chunkinfo.HasVGPU = false;
				chunkinfo.OffsetLocations = chunkinfooffsets;
				vref.ChunkInfos.push_back(chunkinfo);
			}
		}
	}


}

VREFInfo VREF::Read_VREFInfo(BYTES& VREFBYTES, CAFF& caff, bool BigEndian)
{
	VREFInfo vref;

	vref.VDAT_Uncompressed_Size = Vince::ConvertBytesToInt(VREFBYTES, 9, BigEndian);
	vref.VDAT_Compressed_Size = Vince::ConvertBytesToInt(VREFBYTES, 29, BigEndian);

	if (VREFTypeCheck(VREFBYTES)) {
		vref.VGPU_Uncompressed_Size = 0;
		vref.VGPU_Compressed_Size = 0;
		vref.NameBlockOffset = 43 + (caff.header.ChunkCount * 4);
		vref.InfoBlockOffset = Vince::ConvertBytesToInt(VREFBYTES, 39, BigEndian) + vref.NameBlockOffset;
	}
	else {
		vref.VGPU_Uncompressed_Size = Vince::ConvertBytesToInt(VREFBYTES, 42, BigEndian);
		vref.VGPU_Compressed_Size = Vince::ConvertBytesToInt(VREFBYTES, 62, BigEndian);
		vref.NameBlockOffset = 81 + (caff.header.ChunkCount * 4);
		vref.InfoBlockOffset = Vince::ConvertBytesToInt(VREFBYTES, 77, BigEndian) + vref.NameBlockOffset;
	}

	vref.VDAT_Offset = (caff.header.VREF_Compressed_Size + caff.header.HeaderSize) + caff.header.VLUT_Compressed_Size;
	vref.VGPU_Offset = vref.VDAT_Offset + vref.VDAT_Compressed_Size;

	Read_VREF_Chunks(VREFBYTES, caff, vref, BigEndian);

	return vref;
}
