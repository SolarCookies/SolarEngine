#pragma once
#include "Chunks/Chunk.h"
#include "../../../Shaders/Texture.h"

inline std::vector<Texture*> chunkTextures; // caffindex[0].


inline Texture* LookupTexture(int CAFFIndex, int ChunkIndex)
{
    std::string textureName = "CAFF" + std::to_string(CAFFIndex) + "_chunk" + std::to_string(ChunkIndex) + "_texture";
	//std::cout << "Looking up texture: " << textureName << std::endl;
    for (Texture* tex : chunkTextures)
    {
        if (tex->NameOfTexture == textureName) return tex;
    }
    return nullptr;
}

inline Texture* LookupTexture(std::string textureName)
{
    //std::cout << "Looking up texture: " << textureName << std::endl;
    for (Texture* tex : chunkTextures)
    {
        if (tex->NameOfTexture == textureName) return tex;
    }
    return nullptr;
}

inline void AddTexture(int CAFFIndex, int ChunkIndex, Texture* texture)
{
	std::string textureName = "CAFF" + std::to_string(CAFFIndex) + "_chunk" + std::to_string(ChunkIndex) + "_texture";
	//std::cout << "Adding texture: " << textureName << std::endl;
	texture->NameOfTexture = textureName;
	chunkTextures.push_back(texture);
}