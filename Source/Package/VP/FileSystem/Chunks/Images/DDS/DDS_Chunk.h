#pragma once
#include "../../Chunk.h"
#include <math.h>

class DDS_Chunk : public Chunk
{
public:
	std::vector<unsigned char> DDSImage; // readable image without the first 4 bytes viva pinata adds " dds"
	//image handle for thumbnail rendering
    Texture DDSTexture;
    uint16_t SizeX = 0;
    uint16_t SizeY = 0;
	uint32_t Encoding = 0;

#pragma pack(push,1)
    struct DDS_HEADER {
        uint32_t dwSize;
        uint32_t dwFlags;
        uint32_t dwHeight;
        uint32_t dwWidth;
        uint32_t dwPitchOrLinearSize;
        uint32_t dwDepth;
        uint32_t dwMipMapCount;
        uint32_t dwReserved1[11];

        // Pixel format
        struct {
            uint32_t dwSize;
            uint32_t dwFlags;
            uint32_t dwFourCC;
            uint32_t dwRGBBitCount;
            uint32_t dwRBitMask;
            uint32_t dwGBitMask;
            uint32_t dwBBitMask;
            uint32_t dwABitMask;
        } ddspf;

        uint32_t dwCaps;
        uint32_t dwCaps2;
        uint32_t dwCaps3;
        uint32_t dwCaps4;
        uint32_t dwReserved2;
    };
#pragma pack(pop)

    inline uint32_t DetectMipCount(uint32_t width, uint32_t height, uint32_t blockSize, size_t bufferSize)
    {
        uint32_t mipCount = 0;
        size_t offset = 0;
        uint32_t w = width;
        uint32_t h = height;

        while (true) {
            uint32_t blocksX = (w + 3) / 4;
            uint32_t blocksY = (h + 3) / 4;
            size_t levelSize = blocksX * blocksY * blockSize;

            if (offset + levelSize > bufferSize)
                break;

            offset += levelSize;
            mipCount++;

            if (w == 1 && h == 1)
                break;

            w = (std::max)(1u, w >> 1);
            h = (std::max)(1u, h >> 1);
        }

        return mipCount;
    }

    inline std::vector<unsigned char> MakeDDSHeader(uint32_t width, uint32_t height, const char* fourcc, uint32_t mipCount)
    {
        DDS_HEADER header{};
        header.dwSize = 124;
        header.dwFlags = 0x00021007; // CAPS | HEIGHT | WIDTH | PIXELFORMAT | LINEARSIZE
        header.dwHeight = height;
        header.dwWidth = width;
        header.dwPitchOrLinearSize = 0;
        header.dwDepth = 0;
        header.dwMipMapCount = mipCount;

        header.ddspf.dwSize = 32;
        header.ddspf.dwFlags = 0x4;
        header.ddspf.dwFourCC = fourcc[0] | (fourcc[1] << 8) | (fourcc[2] << 16) | (fourcc[3] << 24);

        header.dwCaps = 0x00401008; // TEXTURE | COMPLEX | MIPMAP

        std::vector<unsigned char> dds;
        dds.reserve(128);
        dds.insert(dds.end(), { 'D','D','S',' ' });
        dds.insert(dds.end(), (unsigned char*)&header, (unsigned char*)&header + sizeof(DDS_HEADER));
        return dds;
    }
    
    unsigned int TextureSwizzleDXT3_40AB60(unsigned int a1,unsigned int a2,unsigned int a3,int a4)
    {
        int v4;
        int v5;
        unsigned int v6;

        v4 = (a2 >> 2) + (a2 >> 1 >> (a2 >> 2));
        v5 = ((a3 & 7) + 4 * (a1 & 6)) << ((a2 >> 2) + (a2 >> 1 >> (a2 >> 2)));
        v6 = (v5 & 0xF)
            + ((a1 & 8) << (v4 + 3))
            + (((a3 >> 5) + (a1 >> 5) * ((unsigned int)(a4 + 31) >> 5)) << (v4 + 7))
            + 2 * ((v5 & 0xFFFFFFF0) + 8 * (a1 & 1));
        return ((v6 & 0x3F) + ((8 * ((unsigned char)a3 + 2 * (a1 & 0xF8))) & 0xC0) + 4 * ((v6 & 0x1C0) + 2 * ((v6 & 0xFFFFFE00) + 16 * (a1 & 0x10)))) >> v4;
    }

    std::vector<unsigned char> DeswizzleDXT(
        const std::vector<unsigned char>& src,
        int width, int height, int blockSize)
    {
        int blocksX = width / 4;
        int blocksY = height / 4;
        std::vector<unsigned char> dst(src.size());

        for (int y = 0; y < blocksY; y++) {
            for (int x = 0; x < blocksX; x++) {
                unsigned int blockIndex = TextureSwizzleDXT3_40AB60(y, blockSize, x, blocksX);
                unsigned int srcOffset = blockIndex * blockSize;
                unsigned int dstOffset = (y * blocksX + x) * blockSize;

                for (int i = 0; i < blockSize; i++) {
                    dst[dstOffset + i] = src[srcOffset + (i ^ 1)];
                }
            }
        }
        return dst;
    }
    
	bool hasloaded = false;

    DDS_Chunk() : Chunk() {};
    DDS_Chunk(const std::vector<unsigned char>& rawFile) : Chunk(rawFile) {};
    DDS_Chunk(const std::vector<unsigned char>& rawVDAT, const std::vector<unsigned char>& rawVGPU, bool& IsBig, std::string& name, ChunkInfo& Info) 
        : Chunk(rawVDAT, rawVGPU, IsBig, name, Info) 
    {
        memcpy(&SizeX, &rawVDAT.data()[0] + 8, sizeof(uint16_t));
        memcpy(&SizeY, &rawVDAT.data()[0] + 10, sizeof(uint16_t));
        memcpy(&Encoding, &rawVDAT.data()[0], sizeof(uint32_t));

        char first4[4];
		memcpy(&first4, &rawVGPU.data()[0], sizeof(first4));

        //2E 64 64 73
        if (first4[0] == 0x2E && first4[1] == 0x64 && first4[2] == 0x64 && first4[3] == 0x73) { // " dds" in little endian
            // easy case: skip the fake " dds" and use the header
            DDSImage = std::vector<unsigned char>(VGPU.begin() + 4, VGPU.end());
            if (!ExtractAll) {
                DDSTexture = Texture(DDSImage); //This only works if the data has a header
                hasloaded = true;
            }
        }
        else {
            // harder case: swizzled raw data

            if (Encoding == 1) { // DXT1
                int blockSize = 8;
                auto linear = DeswizzleDXT(VGPU, SizeX, SizeY, blockSize);
                uint32_t mipCount = DetectMipCount(SizeX, SizeY, blockSize, linear.size());
                DDSImage = MakeDDSHeader(SizeX, SizeY, "DXT1",mipCount);
				DDSImage.insert(DDSImage.end(), linear.begin(), linear.end());

                if (!ExtractAll) {
                    DDSTexture = Texture(DDSImage); //This only works if the data has a header
                    hasloaded = true;
                }
            }
            else if (Encoding == 2) { // DXT3
                int blockSize = 16;

                auto linear = DeswizzleDXT(VGPU, SizeX, SizeY, blockSize);

                uint32_t mipCount = DetectMipCount(SizeX, SizeY, blockSize, linear.size());

                DDSImage = MakeDDSHeader(SizeX, SizeY, "DXT3", mipCount);
                DDSImage.insert(DDSImage.end(), linear.begin(), linear.end());

                if (!ExtractAll) {
                    DDSTexture = Texture(DDSImage); //This only works if the data has a header
                    hasloaded = true;
                }
				
            }
            else if (Encoding == 3) { // DXT5
                int blockSize = 16;
                auto linear = DeswizzleDXT(VGPU, SizeX, SizeY, blockSize);

                uint32_t mipCount = DetectMipCount(SizeX, SizeY, blockSize, linear.size());
                DDSImage = MakeDDSHeader(SizeX, SizeY, "DXT5", mipCount);
				DDSImage.insert(DDSImage.end(), linear.begin(), linear.end());

                if (!ExtractAll) {
                    DDSTexture = Texture(DDSImage); //This only works if the data has a header
                    hasloaded = true;
                }
            }

        }
        if (NameWithoutMetadata.find("madeup") != std::string::npos) {
            //generate a name, make a hash of the data or something and append it to aid_texture_madeup
            std::string ddsHash = HashData(DDSImage);
            NameWithoutMetadata = "aid_texture_madeup_" + ddsHash.substr(0, 8); // Use first 8 characters of the hash for uniqueness
        }
		
    };

    void RenderListItem() override {
		//set render dpi to 3x for dds preview
        if (hasloaded) {
            ImGui::Image((ImTextureID)(intptr_t)DDSTexture.ID, ImVec2(50, 50));
			ImGui::SameLine();
        }
        Chunk::RenderListItem();

    };

    void RenderListClick() override {
        if (ImGui::Button("View VDAT")) {
            FileName = NameWithoutMetadata + ".vdat";
            FileViewType = 0;
            if (PendingUpdate) {
                FileViewBuffer = UpdatedVDAT;
            }
            else {
                FileViewBuffer = VDAT;
            }
            GlobalSaveFunction = [this](const std::vector<unsigned char>& data) { this->SaveFile(data); };
        }
        ImGui::SameLine();
        if (ImGui::Button("Export VDAT")) {
            bool saved = Walnut::OpenFileDialog::AskSaveFile(VDAT, Name + ".vdat");
        }
        ImGui::SameLine();
        if (ImGui::Button("Replace VDAT")) {
            //select Patch file
            std::string PatchFilePath = Walnut::OpenFileDialog::OpenFile("VDAT Files\0*.vdat\0\0");
            //Open file and read it into UpdatedVDAT
            if (!PatchFilePath.empty()) {
                std::ifstream file(PatchFilePath, std::ios::binary);
                if (file) {
                    UpdatedVDAT = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                    file.close();
                    PendingUpdate = true;
                }
            }
        }
        if (info.HasGpu)
        {
            if (ImGui::Button("View VGPU")) {
                FileName = NameWithoutMetadata + ".vgpu";
                FileViewType = 1;
                if (PendingUpdate) {
                    FileViewBuffer = UpdatedVGPU;
                }
                else {
                    FileViewBuffer = VGPU;
                }
                GlobalSaveFunction = [this](const std::vector<unsigned char>& data) { this->SaveFile(data); };
            }
            ImGui::SameLine();
            if (ImGui::Button("Export VGPU")) {
                bool saved = Walnut::OpenFileDialog::AskSaveFile(VGPU, Name + ".vgpu");
            }
            ImGui::SameLine();
            if (ImGui::Button("Replace VGPU")) {
                //select Patch file
                std::string PatchFilePath = Walnut::OpenFileDialog::OpenFile("VGPU Files\0*.vgpu\0\0");
                //Open file and read it into UpdatedVGPU
                if (!PatchFilePath.empty()) {
                    std::ifstream file(PatchFilePath, std::ios::binary);
                    if (file) {
                        UpdatedVGPU = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                        file.close();
                        PendingUpdate = true;
                    }
                }
            }
            if (ImGui::Button("View Texture")) {
                PreviewTexture = &DDSTexture;
            }
        }
    };

    void RenderListHover() override {
		Chunk::RenderListHover();
        
        
	};

    ExportInfo ExportChunk() override { //Gets chunk as a common asset format like a dds or fbx returns empty if unknown or non asset file
		std::string NewName = NameWithoutMetadata;
        //if name has madup in it
		if (NameWithoutMetadata.find("madeup") != std::string::npos) {
			//generate a name, make a hash of the data or something and append it to aid_texture_madeup
            std::string ddsHash = HashData(DDSImage);
			NewName = "aid_texture_madeup_" + ddsHash.substr(0, 8); // Use first 8 characters of the hash for uniqueness
        }
		ExportInfo info;
		info.Data = DDSImage;
        info.Name = NewName;
		info.Extension = "dds";

		return info;
    }

    void Destroy() override {
        // Cleanup
		Chunk::Destroy();
		if (PreviewTexture == &DDSTexture) PreviewTexture = nullptr;
		DDSTexture.Delete();
		DDSImage.clear();
    }

};
