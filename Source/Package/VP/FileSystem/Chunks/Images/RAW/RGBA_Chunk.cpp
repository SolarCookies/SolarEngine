#include "RGBA_Chunk.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

ExportInfo RGBA_Chunk::ExportChunk()
{ //Gets chunk as a common asset format like a dds or fbx returns empty if unknown or non asset file 
		//Ill need to convert the raw RGBA data to a png 
	ExportInfo info;

	info.SaveFunction = [this](const std::string& path) { stbi_write_png(path.c_str(), SizeX, SizeY, 4, RGBAImage.data(), SizeX * 4); };

	std::string NewName = NameWithoutMetadata;
	if (NameWithoutMetadata.find("madeup") != std::string::npos) {
		//generate a name, make a hash of the data or something and append it to aid_texture_madeup
		std::string ddsHash = HashData(RGBAImage);
		NewName = "aid_texture_madeup_" + ddsHash.substr(0, 8); // Use first 8 characters of the hash for uniqueness
	}
	info.Data = RGBAImage;
	info.Name = NewName;
	info.Extension = "png";

	return info;
}