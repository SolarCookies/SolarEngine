#pragma once
#include "../Chunk.h"
#include <fbxsdk.h>
#include "../../../Utils/ZLibHelpers.h"
#include "../../../Utils/OpenFileDialog.h"
#include "../../../../../Windows/Log.hpp"



//Located at 0x0 in vdat
struct ModelHeader {
    uint32_t FooterOffset;
    uint32_t NumOfEntrys;
    uint32_t unk1;
    uint32_t unk2;
    uint32_t unk3;
};

struct EntryType {
    uint32_t ID;
    uint32_t Offset;
};

struct ModelFooter {
    std::vector<EntryType> Entrys;

};

struct Rendergraph {
    uint32_t ModelInfoOffset;
    uint32_t Unk1;
    uint32_t Unk2;
    uint32_t Unk3;
    uint32_t TextureNamesOffset;
    uint32_t TextureMapsOffset;
    uint32_t NumberOfTextures;
    uint32_t unk5;
    uint32_t morphTargetTableOffset;
    uint32_t morphTargetTableCount;
    uint32_t inputDefinition1TableOffset;
    uint32_t inputDefinition1TableNumber;
    uint32_t inputDefinitionSize;
    uint32_t inputDefinition2TableOffset;
    uint32_t inputDefinition2TableNumber;
    uint32_t unk10;
    uint32_t shadowMapNames;
    uint32_t NumberOfshadowMaps;
    uint32_t unk13;
    uint32_t unk14; //Offset
    uint32_t unk14Count;
    uint32_t unk16; //Offset
    uint32_t unk16Count;
    uint32_t unk17;
    uint32_t unk18;
    uint32_t unk19;
    uint32_t unk20;
    uint32_t unk21;
    uint32_t unk22;
    uint32_t unk23;
    uint32_t unk24;
    uint32_t unk25;
    uint32_t unk26;
    uint32_t unk27;
    uint32_t unk28;
    uint32_t unk29;
    float unk30;
    float unk31;
    uint32_t unk32;
};

struct ModelInfo {
    uint8_t Type;
    uint8_t Flag2;
    uint16_t Index;
    uint32_t NextOffset;
    uint32_t AdditonalOffset;
    uint32_t LastOffset;
	uint32_t CurrentOffset; //Not in file, just used for tracking where we are
};
#pragma pack(push, 1)
struct ModelVertDef
{
    ModelInfo info;
    uint16_t unk1;
    uint16_t unk2;
    uint32_t vertexCount;
    uint32_t dataOffset;
    uint32_t vertexOffset;
    uint32_t vertTableLength;
    uint16_t entrySize;
    uint16_t unk5;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ModelIndicesDef
{
    ModelInfo info;
    //int unk1;

    // Not entirely sure about this, as some models load the indices fine and others don't.
    int IndicesSize;
    int IndicesCount;
    int unk4;
    int IndicesCount2UNK;
    int unk6;
    int IndicesOffset;
    char unk7[28];
    int IndicesOffset2;
    int IndicesOffset3;
    int IndicesOffset4;
    char unk8[22];
    int IndicesCount2;
	int IndicesCount3; 
};
#pragma pack(pop)

struct ModelBlock {
    ModelVertDef VertDef;
    ModelIndicesDef IndiceDef;
};

struct Model_Viva {
    std::vector<ModelVertDef> verts;
    std::vector<ModelIndicesDef> indices;
    std::vector<Object1> objects;
    Rendergraph RG;
};

struct ModelSection {
    ModelVertDef verts;
    std::vector<ModelIndicesDef> indice;
};

//traverses all model blocks starting from StartOffset and returns a list of all unique offsets found
inline std::vector<int> GetAllBlockOffsets(std::vector<unsigned char> VDAT, int StartOffset, bool BigEndian) {
    std::vector<int> offsets;
    std::vector<int> toVisit;
    toVisit.push_back(StartOffset);

    while (!toVisit.empty()) {

        int currentOffset = toVisit.back();
        toVisit.pop_back();

        // Check if we've already visited this offset
        if (std::find(offsets.begin(), offsets.end(), currentOffset) != offsets.end()) {
            continue; // Already visited
        }

        offsets.push_back(currentOffset);

        ModelInfo mi;

        if (currentOffset + sizeof(ModelInfo) > VDAT.size()) {
            std::cout << "Current offset exceeds VDAT size, stopping traversal." << std::endl;
            break;
        }

        memcpy(&mi, &VDAT.data()[currentOffset], sizeof(ModelInfo));
        if (BigEndian) {
            mi.NextOffset = _byteswap_ulong(mi.NextOffset);
            mi.AdditonalOffset = _byteswap_ulong(mi.AdditonalOffset);
            mi.LastOffset = _byteswap_ulong(mi.LastOffset);
            mi.Index = _byteswap_ushort(mi.Index);
        }

        // Queue both Next and Additional offsets if they are valid and not already visited
        if (mi.NextOffset != 0 && std::find(offsets.begin(), offsets.end(), mi.NextOffset) == offsets.end()) {
            toVisit.push_back(mi.NextOffset);
        }
        if (mi.AdditonalOffset != 0 && std::find(offsets.begin(), offsets.end(), mi.AdditonalOffset) == offsets.end()) {
            toVisit.push_back(mi.AdditonalOffset);
        }
    }
    return offsets;
}

inline std::vector<ModelSection> GetAllModelSections(std::vector<unsigned char> VDAT, std::vector<int> BlockOffsets, bool BigEndian) {

	std::vector<ModelInfo> ModelInfos;
    for(int offset : BlockOffsets) {
        ModelInfo mi;
        memcpy(&mi, &VDAT.data()[offset], sizeof(ModelInfo));
        if (BigEndian) {
            mi.NextOffset = _byteswap_ulong(mi.NextOffset);
            mi.AdditonalOffset = _byteswap_ulong(mi.AdditonalOffset);
            mi.LastOffset = _byteswap_ulong(mi.LastOffset);
            mi.Index = _byteswap_ushort(mi.Index);
        }
        mi.CurrentOffset = offset; // Store where we found this block for reference
		ModelInfos.push_back(mi);
	}

    std::vector<ModelInfo> SortedModelInfos;

	//use Index in model info to sort the blocks into the correct order
	SortedModelInfos.resize(ModelInfos.size());
    for (const ModelInfo& mi : ModelInfos) {
        if (mi.Index < SortedModelInfos.size()) {
            SortedModelInfos[mi.Index] = mi;
        }
    }

	//Models start with vertex definition (type 2) and are followed by multiple indice definitions (type 6 or 7) once we hit another type 2 that is the begining of a new model
	std::vector<ModelSection> ModelSections;
    ModelSection currentSection;
    bool inSection = false;
    for (const ModelInfo& mi : SortedModelInfos) {
        if (mi.Type == 2) {
            // If we were already in a section, save it before starting a new one
            if (inSection) {
                ModelSections.push_back(currentSection);
                currentSection = ModelSection(); // Reset for the new section
            }
            // Start a new section with the vertex definition
            memcpy(&currentSection.verts, &VDAT.data()[mi.CurrentOffset], sizeof(ModelVertDef));
            if (BigEndian) {
                currentSection.verts.info.NextOffset = _byteswap_ulong(currentSection.verts.info.NextOffset);
                currentSection.verts.info.AdditonalOffset = _byteswap_ulong(currentSection.verts.info.AdditonalOffset);
                currentSection.verts.info.LastOffset = _byteswap_ulong(currentSection.verts.info.LastOffset);
                currentSection.verts.info.Index = _byteswap_ushort(currentSection.verts.info.Index);
                currentSection.verts.vertexCount = _byteswap_ulong(currentSection.verts.vertexCount);
                currentSection.verts.dataOffset = _byteswap_ulong(currentSection.verts.dataOffset);
                currentSection.verts.vertexOffset = _byteswap_ulong(currentSection.verts.vertexOffset);
                currentSection.verts.vertTableLength = _byteswap_ulong(currentSection.verts.vertTableLength);
                currentSection.verts.entrySize = _byteswap_ushort(currentSection.verts.entrySize);
                currentSection.verts.unk1 = _byteswap_ushort(currentSection.verts.unk1);
                currentSection.verts.unk2 = _byteswap_ushort(currentSection.verts.unk2);
                currentSection.verts.unk5 = _byteswap_ushort(currentSection.verts.unk5);
            }
            inSection = true;
        }
        else if ((mi.Type == 6 || mi.Type == 7) && inSection) {
            // Add indice definitions to the current section
            ModelIndicesDef indice;
            memcpy(&indice, &VDAT.data()[mi.CurrentOffset], sizeof(ModelIndicesDef));
            if (BigEndian) {
                indice.info.NextOffset = _byteswap_ulong(indice.info.NextOffset);
                indice.info.AdditonalOffset = _byteswap_ulong(indice.info.AdditonalOffset);
                indice.info.LastOffset = _byteswap_ulong(indice.info.LastOffset);
                indice.info.Index = _byteswap_ushort(indice.info.Index);
                indice.IndicesSize = _byteswap_ulong(indice.IndicesSize);
                indice.IndicesCount = _byteswap_ulong(indice.IndicesCount);
                indice.IndicesCount2UNK = _byteswap_ulong(indice.IndicesCount2UNK);
                indice.IndicesOffset = _byteswap_ulong(indice.IndicesOffset);
                indice.IndicesOffset2 = _byteswap_ulong(indice.IndicesOffset2);
                indice.IndicesOffset3 = _byteswap_ulong(indice.IndicesOffset3);
                indice.IndicesOffset4 = _byteswap_ulong(indice.IndicesOffset4);
                indice.IndicesCount2 = _byteswap_ulong(indice.IndicesCount2);
                indice.IndicesCount3 = _byteswap_ulong(indice.IndicesCount3);
                indice.unk4 = _byteswap_ulong(indice.unk4);
                indice.unk6 = _byteswap_ulong(indice.unk6);
            }
            currentSection.indice.push_back(indice);
        }
        // If we encounter other types, we ignore them for now
    }
    // Don't forget to add the last section if we were in one
    if (inSection) {
        ModelSections.push_back(currentSection);
    }
	return ModelSections;
}


class Model_Chunk : public Chunk
{
private:
    int visitedOffsets[1024];
    int visitedCount = 0;
    int offsetsToVisit[1024];
    int offsetCount = 0;

public:
    Model_Viva MODEL;

private:
    void exportFBX(
        const std::vector<Object1>& object,
        const std::string& filename)
    {
        // 1. Initialize SDK Manager and Scene
        FbxManager* manager = FbxManager::Create();
        FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
        manager->SetIOSettings(ios);

        FbxScene* scene = FbxScene::Create(manager, "MyScene");
        FbxNode* rootNode = scene->GetRootNode();

        // 2. Loop over objects
        for (size_t objIndex = 0; objIndex < object.size(); ++objIndex) {
            const auto& verts = object[objIndex].objectsVerts;
			const auto& indices = object[objIndex].objectsIndices;

            // Create mesh + node
            std::string meshName = "Mesh_" + std::to_string(objIndex);
            FbxMesh* mesh = FbxMesh::Create(scene, meshName.c_str());
            FbxNode* node = FbxNode::Create(scene, meshName.c_str());
            node->SetNodeAttribute(mesh);
            rootNode->AddChild(node);

            // Allocate control points
            mesh->InitControlPoints((int)verts.size());
            FbxVector4* ctrlPoints = mesh->GetControlPoints();
            for (size_t i = 0; i < verts.size(); i++) {
                ctrlPoints[i] = FbxVector4(verts[i].position.x,
                    verts[i].position.y,
                    verts[i].position.z);
            }

            // Normals
            FbxLayer* layer = mesh->GetLayer(0);
            if (!layer) {
                mesh->CreateLayer();
                layer = mesh->GetLayer(0);
            }

            FbxLayerElementNormal* normalElement = FbxLayerElementNormal::Create(mesh, "");
            normalElement->SetMappingMode(FbxLayerElement::eByControlPoint);
            normalElement->SetReferenceMode(FbxLayerElement::eDirect);
            for (const auto& v : verts) {
                normalElement->GetDirectArray().Add(FbxVector4(v.normal.x, v.normal.y, v.normal.z));
            }
            layer->SetNormals(normalElement);

            // UVs
            FbxLayerElementUV* uvElement = FbxLayerElementUV::Create(mesh, "UVSet");
            uvElement->SetMappingMode(FbxLayerElement::eByControlPoint);
            uvElement->SetReferenceMode(FbxLayerElement::eDirect);
            for (const auto& v : verts) {
                uvElement->GetDirectArray().Add(FbxVector2(v.texCoord.u, v.texCoord.v));
            }
            layer->SetUVs(uvElement, FbxLayerElement::eTextureDiffuse);

            // Faces (triangles)
            for (size_t i = 0; i < indices.size(); i += 3) {
                if (i + 2 < indices.size()) {
                    mesh->BeginPolygon();
                    mesh->AddPolygon(indices[i]);
                    mesh->AddPolygon(indices[i + 1]);
                    mesh->AddPolygon(indices[i + 2]);
                    mesh->EndPolygon();
                }
            }
        }

        // 3. Export to FBX file
        int fileFormat = manager->GetIOPluginRegistry()->GetNativeWriterFormat();
        FbxExporter* exporter = FbxExporter::Create(manager, "");
        if (!exporter->Initialize(filename.c_str(), fileFormat, manager->GetIOSettings())) {
            throw std::runtime_error("Failed to initialize FBX exporter");
        }

        exporter->Export(scene);
        exporter->Destroy();

        // 4. Cleanup
        manager->Destroy();
    }

    void exportOBJ(std::vector<Vertex1> verts, std::vector<uint32_t> indices, std::string& filename)
    {
        std::ofstream file(filename);
        if (!file) {
            throw std::runtime_error("Failed to open file for writing");
        }

        // Write vertex positions
        for (const auto& v : verts) {
            file << "v " << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
        }

        // Write vertex normals
        for (const auto& v : verts) {
            file << "vn " << v.normal.x << " " << v.normal.y << " " << v.normal.z << "\n";
        }
        // Write texture coordinates
        for (const auto& v : verts) {
            file << "vt " << v.texCoord.u << " " << v.texCoord.v << "\n";
        }

        // Write faces (OBJ is 1-based indexing)
        for (size_t i = 0; i < indices.size(); i += 3) {
            if (i + 2 < indices.size()) {
                uint32_t i1 = indices[i] + 1;
                uint32_t i2 = indices[i + 1] + 1;
                uint32_t i3 = indices[i + 2] + 1;

                // Position/UV/Normal � assuming they share the same index
                file << "f "
                    << i1 << "/" << i1 << "/" << i1 << " "
                    << i2 << "/" << i2 << "/" << i2 << " "
                    << i3 << "/" << i3 << "/" << i3 << "\n";
            }
        }
        file.close();
    }

    std::vector<unsigned char> ReplaceVertsInVGPU(std::vector<unsigned char>& VGPU, ModelVertDef& VertBlock, std::vector<Vector3>& newVerts) {
        std::vector<unsigned char> newVGPU = VGPU; // Start with a copy of the original VGPU data
        // Ensure we don't exceed the original vertex count
        size_t count = (std::min)(newVerts.size(), static_cast<size_t>(VertBlock.vertexCount));
        for (size_t i = 0; i < count; i++) {
            size_t offset = VertBlock.vertexOffset + i * VertBlock.entrySize;
            if (offset + sizeof(Vector3) <= newVGPU.size()) { // Ensure we don't write out of bounds
                // Directly copy the new vertex data (little-endian)
                std::memcpy(&newVGPU[offset], &newVerts.data()[i], sizeof(Vector3));
            }
            else {
                throw std::runtime_error("Attempting to write out of bounds in VGPU data");
            }
        }
        return newVGPU;
    }

    Rendergraph GetRendergraph(std::vector<unsigned char> VDAT, bool IsBigEndian) {
        ModelHeader Header;
        memcpy(&Header, &VDAT.data()[0], sizeof(ModelHeader));

        if (IsBigEndian) {
            Header.FooterOffset = _byteswap_ulong(Header.FooterOffset);
            Header.NumOfEntrys = _byteswap_ulong(Header.NumOfEntrys);
            Header.unk1 = _byteswap_ulong(Header.unk1);
            Header.unk2 = _byteswap_ulong(Header.unk2);
            Header.unk3 = _byteswap_ulong(Header.unk3);
        }

        ModelFooter Footer;
        Footer.Entrys.resize(Header.NumOfEntrys);

        for (int i = 0; i < Header.NumOfEntrys; i++) {
            memcpy(&Footer.Entrys[i], &VDAT.data()[Header.FooterOffset + (i * sizeof(EntryType))], sizeof(EntryType));
            if (IsBigEndian) {
                Footer.Entrys[i].ID = _byteswap_ulong(Footer.Entrys[i].ID);
                Footer.Entrys[i].Offset = _byteswap_ulong(Footer.Entrys[i].Offset);
            }
        }

        MODEL.RG;
        for (const auto& entry : Footer.Entrys) {
            if (entry.ID == 0) { //Rendergraph
                memcpy(&MODEL.RG, &VDAT.data()[entry.Offset], sizeof(Rendergraph));
                if (IsBigEndian) {
                    MODEL.RG.ModelInfoOffset = _byteswap_ulong(MODEL.RG.ModelInfoOffset);
                    MODEL.RG.Unk1 = _byteswap_ulong(MODEL.RG.Unk1);
                    MODEL.RG.Unk2 = _byteswap_ulong(MODEL.RG.Unk2);
                    MODEL.RG.Unk3 = _byteswap_ulong(MODEL.RG.Unk3);
                    MODEL.RG.TextureNamesOffset = _byteswap_ulong(MODEL.RG.TextureNamesOffset);
                    MODEL.RG.TextureMapsOffset = _byteswap_ulong(MODEL.RG.TextureMapsOffset);
                    MODEL.RG.NumberOfTextures = _byteswap_ulong(MODEL.RG.NumberOfTextures);
                    MODEL.RG.unk5 = _byteswap_ulong(MODEL.RG.unk5);
                    MODEL.RG.morphTargetTableOffset = _byteswap_ulong(MODEL.RG.morphTargetTableOffset);
                    MODEL.RG.morphTargetTableCount = _byteswap_ulong(MODEL.RG.morphTargetTableCount);
                    MODEL.RG.inputDefinition1TableOffset = _byteswap_ulong(MODEL.RG.inputDefinition1TableOffset);
                    MODEL.RG.inputDefinition1TableNumber = _byteswap_ulong(MODEL.RG.inputDefinition1TableNumber);
                    MODEL.RG.inputDefinitionSize = _byteswap_ulong(MODEL.RG.inputDefinitionSize);
                    MODEL.RG.inputDefinition2TableOffset = _byteswap_ulong(MODEL.RG.inputDefinition2TableOffset);
                    MODEL.RG.inputDefinition2TableNumber = _byteswap_ulong(MODEL.RG.inputDefinition2TableNumber);
                    MODEL.RG.unk10 = _byteswap_ulong(MODEL.RG.unk10);
                    MODEL.RG.shadowMapNames = _byteswap_ulong(MODEL.RG.shadowMapNames);
                    MODEL.RG.NumberOfshadowMaps = _byteswap_ulong(MODEL.RG.NumberOfshadowMaps);
                    MODEL.RG.unk13 = _byteswap_ulong(MODEL.RG.unk13);
                    MODEL.RG.unk14 = _byteswap_ulong(MODEL.RG.unk14);
                    MODEL.RG.unk14Count = _byteswap_ulong(MODEL.RG.unk14Count);
                    MODEL.RG.unk16 = _byteswap_ulong(MODEL.RG.unk16);
                    MODEL.RG.unk16Count = _byteswap_ulong(MODEL.RG.unk16Count);
                    MODEL.RG.unk17 = _byteswap_ulong(MODEL.RG.unk17);
                    MODEL.RG.unk18 = _byteswap_ulong(MODEL.RG.unk18);
                    MODEL.RG.unk19 = _byteswap_ulong(MODEL.RG.unk19);
                    MODEL.RG.unk20 = _byteswap_ulong(MODEL.RG.unk20);
                    MODEL.RG.unk21 = _byteswap_ulong(MODEL.RG.unk21);
                    MODEL.RG.unk22 = _byteswap_ulong(MODEL.RG.unk22);
                    MODEL.RG.unk23 = _byteswap_ulong(MODEL.RG.unk23);
                    MODEL.RG.unk24 = _byteswap_ulong(MODEL.RG.unk24);
                    MODEL.RG.unk25 = _byteswap_ulong(MODEL.RG.unk25);
                    MODEL.RG.unk26 = _byteswap_ulong(MODEL.RG.unk26);
                    MODEL.RG.unk27 = _byteswap_ulong(MODEL.RG.unk27);
                    MODEL.RG.unk28 = _byteswap_ulong(MODEL.RG.unk28);
                    MODEL.RG.unk29 = _byteswap_ulong(MODEL.RG.unk29);
                    MODEL.RG.unk30 = byteswap_float(MODEL.RG.unk30);
                    MODEL.RG.unk31 = byteswap_float(MODEL.RG.unk31);
                    MODEL.RG.unk32 = _byteswap_ulong(MODEL.RG.unk32);
                }

                break;
            }
        }
        return MODEL.RG;

    }

    
    void LoadModelNew(std::vector<unsigned char> VDAT, std::vector<unsigned char> VGPU, bool BigEndian)
    {
        if(VGPU.size() == 0) {
            return;
		}
        if (ExtractAll) return; //Skip loading model if we are extracting all files (Saves time and atm this isnt stable enough to always run)
        MODEL.RG = GetRendergraph(VDAT, BigEndian);

		//Get First ModelOffset from Rendergraph useally type 0 block or type 5 block
		int CurrentOffset = MODEL.RG.ModelInfoOffset;
		
		std::string ColorName = "CAFF" + std::to_string(CAFFIndex) + "_chunk" + std::to_string(info.VDat.ID + 1) + "_texture";
		std::string NormalName = "CAFF" + std::to_string(CAFFIndex) + "_chunk" + std::to_string(info.VDat.ID + 2) + "_texture"; //normals are usually after the color texture

		std::vector<int> BlockOffsets = GetAllBlockOffsets(VDAT, CurrentOffset, BigEndian);
		std::vector<ModelSection> ModelSections = GetAllModelSections(VDAT, BlockOffsets, BigEndian);

        
        for(const ModelSection& sec : ModelSections) {
               
            std::vector<Vertex1> vertices1;

            vertices1.resize(sec.verts.vertexCount);

		    int VertexBlockSize = sec.verts.entrySize * sec.verts.vertexCount;

            std::vector<unsigned char> vertexData;
            vertexData.resize(VertexBlockSize);
            memcpy(vertexData.data(), &VGPU.data()[sec.verts.vertexOffset], VertexBlockSize);

			    
            for (uint32_t i = 0; i < sec.verts.vertexCount; i++) {
                Vertex1 v;

                std::vector<unsigned char> VertBlockData;
                VertBlockData.resize(sec.verts.entrySize);

                memcpy(VertBlockData.data(), &vertexData[i * sec.verts.entrySize], sec.verts.entrySize);

                VertexBlock block = ConstructVertexBlockFromSize(sec.verts.entrySize, BigEndian, VertBlockData);
                v.position = block.position;
                v.normal = block.normal;
                v.texCoord.u = block.texCoord.u;
                v.texCoord.v = block.texCoord.v * -1;
			    v.extraData = block;
                vertices1[i] = v;
            }

			std::vector<unsigned char> indiceData; //Holds the raw indice data from VGPU
			std::vector<GLuint> indices1; // this is the combined indice list
            for (const ModelIndicesDef& ind : sec.indice) {
                std::vector<unsigned char> indiceDataTemp; //Holds the raw indice data from VGPU

				//figure out what offsets are valid to use, If the offset is 0 it is not used 
                if(ind.IndicesOffset != 0) {
                    if (ind.IndicesOffset + ind.IndicesCount * 3 * 2 > VGPU.size()) {
                        continue;
					}
                    for(int i = 0; i < ind.IndicesCount * 3; i++) {
                        //add indices to indiceData
                        uint16 index1 = 0;
                        memcpy(&index1, &VGPU.data()[ind.IndicesOffset + i * 2], sizeof(uint16));
                        if (BigEndian) index1 = _byteswap_ushort(index1);
                        indices1.push_back(static_cast<GLuint>(index1));
					}
					indiceDataTemp.resize(ind.IndicesCount * 3 * 2);
					memcpy(indiceDataTemp.data(), &VGPU.data()[ind.IndicesOffset], ind.IndicesCount * 3 * 2);
				    indiceData.insert(indiceData.end(), indiceDataTemp.begin(), indiceDataTemp.end());
                }
				
                else if (ind.IndicesOffset2 != 0) {
                    if (ind.IndicesOffset2 + ind.IndicesCount2 * 2 > VGPU.size()) {
                        continue;
                    }
                    for (int i = 0; i < ind.IndicesCount2; i++) {
                        //add indices to indiceData
                        uint16 index1 = 0;
                        memcpy(&index1, &VGPU.data()[ind.IndicesOffset2 + i * 2], sizeof(uint16));
                        if (BigEndian) index1 = _byteswap_ushort(index1);
                        indices1.push_back(static_cast<GLuint>(index1));
                    }
                    indiceDataTemp.resize(ind.IndicesCount2 * 2);
                    memcpy(indiceDataTemp.data(), &VGPU.data()[ind.IndicesOffset2], ind.IndicesCount2 * 2);
                    indiceData.insert(indiceData.end(), indiceDataTemp.begin(), indiceDataTemp.end());
                }

                else if (ind.IndicesOffset3 != 0) {
                    if (ind.IndicesOffset3 + ind.IndicesCount3 * 2 > VGPU.size()) {
                        continue;
                    }
                    for (int i = 0; i < ind.IndicesCount3/3; i++) {
                        //add indices to indiceData
                        uint16 index1 = 0;
                        memcpy(&index1, &VGPU.data()[ind.IndicesOffset3 + i * 2], sizeof(uint16));
                        if (BigEndian) index1 = _byteswap_ushort(index1);
                        indices1.push_back(static_cast<GLuint>(index1));
                    }
                    indiceDataTemp.resize(ind.IndicesCount3*2);
                    memcpy(indiceDataTemp.data(), &VGPU.data()[ind.IndicesOffset3], ind.IndicesCount3*2);
                    indiceData.insert(indiceData.end(), indiceDataTemp.begin(), indiceDataTemp.end());
				    }
                else if (ind.IndicesOffset4 != 0) {
                    if (ind.IndicesOffset4 + ind.IndicesCount2 * 3 * 2 > VGPU.size()) {
                        continue;
                    }
                    //use count2 *3 for indice count
                    for(int i = 0; i < ind.IndicesCount2; i++) {
                        //add indices to indiceData
						uint16 index1 = 0;
						memcpy(&index1, &VGPU.data()[ind.IndicesOffset4 + i * 2], sizeof(uint16));
						if (BigEndian) index1 = _byteswap_ushort(index1);
						indices1.push_back(static_cast<GLuint>(index1));
					}
                    indiceDataTemp.resize(ind.IndicesCount2 * 3 * 2);
                    memcpy(indiceDataTemp.data(), &VGPU.data()[ind.IndicesOffset4], ind.IndicesCount2 * 3 * 2);
                    indiceData.insert(indiceData.end(), indiceDataTemp.begin(), indiceDataTemp.end());
                }
            }
            Object1 obj;
			obj.objectsVerts = vertices1;
			obj.objectsIndices = indices1;
			obj.VertexSize = sec.verts.entrySize;
			obj.rawIndexBlock = indiceData;
			obj.rawVertBlock = vertexData;
            obj.ColorTextureName = ColorName;
			obj.NormalTextureName = NormalName;
		    MODEL.objects.push_back(obj);

        }
            
        
    };

    public:

    Model_Chunk() : Chunk() {};
    Model_Chunk(const std::vector<unsigned char>& rawFile) : Chunk(rawFile) {};
    Model_Chunk(const std::vector<unsigned char>& rawVDAT, const std::vector<unsigned char>& rawVGPU, bool& IsBig, std::string& name, ChunkInfo& Info, int cAFFIndex)
        : Chunk(rawVDAT, rawVGPU, IsBig, name, Info, cAFFIndex)
    {
        
		LoadModelNew(rawVDAT, rawVGPU, IsBig);
    };

    void RenderListHover() override { //Used when drawing imgui list entry hover
        ImGui::BeginTooltip();
        ImGui::Text("ID: %d", info.VDat.ID);
        ImGui::Text("VDAT Offset: %d", info.VDat.Offset);
        ImGui::Text("VDAT Size: %d", info.VDat.Size);
        if (info.HasGpu)
        {
            ImGui::Text("VGPU Offset: %d", info.VGpu.Offset);
            ImGui::Text("VGPU Size: %d", info.VGpu.Size);
        }
        ImGui::Text("File Type: %.2f", FileType);
        //convert timestamp to human readable format
        if (FileTimestamp != 0) {
            char buffer[80];
            tm* timeinfo = localtime(&FileTimestamp);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
            ImGui::Text("OG Timestamp: %s", buffer);
        }
        else {
            ImGui::Text("Timestamp: N/A");
        }

		int numberverts = 0;
		for (int i = 0; i < MODEL.objects.size(); i++) {
			numberverts += MODEL.objects[i].objectsVerts.size();
		}
		ImGui::Text("Number of Verts: %s", std::to_string(numberverts).c_str());

		int numberindices = 0;
        for (int i = 0; i < MODEL.objects.size(); i++) {
            numberindices += MODEL.objects[i].objectsIndices.size();
		}
		ImGui::Text("Number of Indices: %s", std::to_string(numberindices).c_str());
		

		ImGui::Text("Number of Objects: %s", std::to_string(MODEL.objects.size()).c_str());
        ImGui::EndTooltip();
    }

    void RenderListClick() override { //Used when drawing imgui list entry
        Chunk::RenderListClick();
        if (ImGui::Button("Try to Export Model as FBX")) {
            std::string FileName1 = NameWithoutMetadata + ".fbx";
			std::string exportfolder = Walnut::OpenFileDialog::OpenFolder();

            if(!exportfolder.empty()) {
                FileName1 = exportfolder + "/" + FileName1;
                exportFBX(MODEL.objects, FileName1);
			}
        }
        if (ImGui::Button("Try to view Model in Viewport")) {
			CurrentModel = MODEL.objects;
			CurrentModelName = NameWithoutMetadata;
        }
	}
};