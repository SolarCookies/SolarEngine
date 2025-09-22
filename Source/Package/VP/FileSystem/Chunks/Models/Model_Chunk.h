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

		//Get First ModelOffset from Rendergraph
		int CurrentOffset = MODEL.RG.ModelInfoOffset;
		ModelInfo miRoot;
		memcpy(&miRoot, &VDAT.data()[CurrentOffset], sizeof(ModelInfo));
		miRoot.CurrentOffset = CurrentOffset; //Store where we found this block for reference
		CurrentOffset = miRoot.NextOffset;

        ModelInfo TestRoot;
		memcpy(&TestRoot, &VDAT.data()[CurrentOffset], sizeof(ModelInfo)); //check first model block for type 1 (root)

        if(TestRoot.Type != 1) {
            std::cout << "Warning: Expected root model info type 1 but found type " << std::to_string(TestRoot.Type) << ". This may cause issues." << std::endl;
			std::cout << "Trying to find the first type 1 block..." << std::endl;
			int numtries = 0;
			int maxtries = 10; //Prevent infinite loops
            while (TestRoot.Type != 1 && numtries < maxtries) {
				numtries++;
                ModelInfo miCheck;
                memcpy(&miCheck, &VDAT.data()[CurrentOffset], sizeof(ModelInfo));
                if (miCheck.Type == 1) {
                    TestRoot = miCheck;
					TestRoot.CurrentOffset = CurrentOffset; //Store where we found this block for reference
                    std::cout << "Found type 1 block at offset " << std::to_string(CurrentOffset) << std::endl;
                    CurrentOffset = TestRoot.NextOffset;
                    break;
                }
                CurrentOffset = miCheck.NextOffset;
			}
			CurrentOffset = TestRoot.CurrentOffset;

            if(numtries >= maxtries) {
                std::cout << "Error: Could not find a type 1 model info block after " << std::to_string(maxtries) << " tries. Aborting model load." << std::endl;
				std::cerr << "Model root not found for: " << NameWithoutMetadata << std::endl;
                return;
			}
		}

		std::vector<ModelInfo> modelblocks; //Holds all the roots for each model aka type 01

		//Each 01 model info block has a additional offset to the next object in the model until the last one which has 0
        while (CurrentOffset != 0) {
            ModelInfo mi;
            memcpy(&mi, &VDAT.data()[CurrentOffset], sizeof(ModelInfo));
			modelblocks.push_back(mi);
			std::cout << "Found root block of type " << std::to_string(mi.Type) << " at offset " << std::to_string(CurrentOffset) << std::endl;
            CurrentOffset = mi.AdditonalOffset;
        }

		std::vector<std::vector<ModelInfo>> Objects; //Holds all the objects which each have multiple modelblocks (vert + indice definitions)

        for (ModelInfo& mi : modelblocks) {
            std::vector<ModelInfo> Objectblocks;
			Objectblocks.push_back(mi); //Add the root modelblock to the object
			mi.CurrentOffset = CurrentOffset; //Store where we found this block for reference
            CurrentOffset = mi.NextOffset;
            while (CurrentOffset != 0)
            {
                ModelInfo mi2;
                memcpy(&mi2, &VDAT.data()[CurrentOffset], sizeof(ModelInfo));
                mi2.CurrentOffset = CurrentOffset; //Store where we found this block for reference
                Objectblocks.push_back(mi2);
				std::cout << "Found sub-block of type " << std::to_string(mi2.Type) << " at offset " << std::to_string(CurrentOffset) << std::endl;
                CurrentOffset = mi2.NextOffset;
            }
			Objects.push_back(Objectblocks);

			//Load the one 02 type (Vertex Definition) and all of the 06 types (Indice Definition)
			int VertTypesFound = 0;
			int IndiceTypesFound = 0;


            //Load Object data
            ModelVertDef vert;
            std::vector<ModelIndicesDef> indice; //models can have multiple face blocks
            for(const ModelInfo& block : Objectblocks) {

                if (block.Type == 2){
                    VertTypesFound++;
                    memcpy(&vert, &VDAT.data()[block.CurrentOffset], sizeof(ModelVertDef));
                }


                if (block.Type == 6 || block.Type == 7) {
                    IndiceTypesFound++;
					ModelIndicesDef ind;
					memcpy(&ind, &VDAT.data()[block.CurrentOffset], sizeof(ModelIndicesDef));
					indice.push_back(ind);
                }
			}
            std::cout << "Vertex Count: " << std::to_string(vert.vertexCount) << std::endl;
			std::cout << "Vertex size: " << std::to_string(vert.entrySize) << std::endl;
            std::cout << "_______________________________________________________________________________" << std::endl;
			std::cout << "Vertex offset: " << std::to_string(vert.vertexOffset) << std::endl;
			std::cout << "Vertex Table Length: " << std::to_string(vert.vertTableLength) << std::endl;
            std::cout << "_______________________________________________________________________________" << std::endl;
            std::vector<Vertex1> vertices1;

            vertices1.resize(vert.vertexCount);

			int VertexBlockSize = vert.entrySize * vert.vertexCount;

            std::vector<unsigned char> vertexData;
            vertexData.resize(VertexBlockSize);
            memcpy(vertexData.data(), &VGPU.data()[vert.vertexOffset], VertexBlockSize);

			//convert object data into a usable format
            if (VertTypesFound != 1) {
                std::cout << "Warning: Found " << std::to_string(VertTypesFound) << " vertex definitions in object, expected 1. This may cause issues." << std::endl;
            }
            else {
                for (uint32_t i = 0; i < vert.vertexCount; i++) {
                    Vertex1 v;

                    std::vector<unsigned char> VertBlockData;
                    VertBlockData.resize(vert.entrySize);

                    memcpy(VertBlockData.data(), &vertexData[i * vert.entrySize], vert.entrySize);

                    VertexBlock block = ConstructVertexBlockFromSize(vert.entrySize, BigEndian, VertBlockData);
                    v.position = block.position;
                    v.normal = block.normal;
                    v.texCoord = block.texCoord;
                    vertices1[i] = v;
                }
            }

			std::vector<unsigned char> indiceData; //Holds the raw indice data from VGPU
			std::vector<GLuint> indices1; // this is the combined indice list
            for (const ModelIndicesDef& ind : indice) {
                std::vector<unsigned char> indiceDataTemp; //Holds the raw indice data from VGPU

				//figure out what offsets are valid to use, If the offset is 0 it is not used 
                if(ind.IndicesOffset != 0) {
                    for(int i = 0; i < ind.IndicesCount * 3; i++) {
                        //add indices to indiceData
                        uint16 index1 = 0;
                        memcpy(&index1, &VGPU.data()[ind.IndicesOffset + i * 2], sizeof(uint16));
                        indices1.push_back(static_cast<GLuint>(index1));
					}
					indiceDataTemp.resize(ind.IndicesCount * 3 * 2);
					memcpy(indiceDataTemp.data(), &VGPU.data()[ind.IndicesOffset], ind.IndicesCount * 3 * 2);
					indiceData.insert(indiceData.end(), indiceDataTemp.begin(), indiceDataTemp.end());
                }
				
                else if (ind.IndicesOffset2 != 0) {
                    for (int i = 0; i < ind.IndicesCount2; i++) {
                        //add indices to indiceData
                        uint16 index1 = 0;
                        memcpy(&index1, &VGPU.data()[ind.IndicesOffset2 + i * 2], sizeof(uint16));
                        
                        indices1.push_back(static_cast<GLuint>(index1));
                    }
                    indiceDataTemp.resize(ind.IndicesCount2 * 2);
                    memcpy(indiceDataTemp.data(), &VGPU.data()[ind.IndicesOffset2], ind.IndicesCount2 * 2);
                    indiceData.insert(indiceData.end(), indiceDataTemp.begin(), indiceDataTemp.end());
                    std::cout << "_______________________________________________________________________________" << std::endl;
					std::cout << "Indice block 2 used." << std::endl;
					std::cout << "Indice Count: " << std::to_string(ind.IndicesCount2) << std::endl;
					std::cout << "Indice Offset: " << std::to_string(ind.IndicesOffset2) << std::endl;
                    std::cout << "Indice Block Size: " << std::to_string(ind.IndicesCount2 * 2) << std::endl;
                    std::cout << "_______________________________________________________________________________" << std::endl;
                    
                }

                else if (ind.IndicesOffset3 != 0) {
                    for (int i = 0; i < ind.IndicesCount3/3; i++) {
                        //add indices to indiceData
                        uint16 index1 = 0;
                        memcpy(&index1, &VGPU.data()[ind.IndicesOffset3 + i * 2], sizeof(uint16));
                        indices1.push_back(static_cast<GLuint>(index1));
                    }
                    indiceDataTemp.resize(ind.IndicesCount3*2);
                    memcpy(indiceDataTemp.data(), &VGPU.data()[ind.IndicesOffset3], ind.IndicesCount3*2);
                    indiceData.insert(indiceData.end(), indiceDataTemp.begin(), indiceDataTemp.end());
                    std::cout << "_______________________________________________________________________________" << std::endl;
					std::cout << "Indice block 3 used." << std::endl;
					std::cout << "Indice Count: " << std::to_string(ind.IndicesCount3) << std::endl;
					std::cout << "Indice Offset: " << std::to_string(ind.IndicesOffset3) << std::endl;
					std::cout << "Indice Block Size: " << std::to_string(ind.IndicesCount3 * 2) << std::endl;
                    std::cout << "_______________________________________________________________________________" << std::endl;
				}
                else if (ind.IndicesOffset4 != 0) {
                    //use count2 *3 for indice count
                    for(int i = 0; i < ind.IndicesCount2; i++) {
                        //add indices to indiceData
						uint16 index1 = 0;
						memcpy(&index1, &VGPU.data()[ind.IndicesOffset4 + i * 2], sizeof(uint16));
						indices1.push_back(static_cast<GLuint>(index1));
					}
                    indiceDataTemp.resize(ind.IndicesCount2 * 3 * 2);
                    memcpy(indiceDataTemp.data(), &VGPU.data()[ind.IndicesOffset4], ind.IndicesCount2 * 3 * 2);
                    indiceData.insert(indiceData.end(), indiceDataTemp.begin(), indiceDataTemp.end());
                    std::cout << "_______________________________________________________________________________" << std::endl;
					std::cout << "Indice block 4 used." << std::endl;
					std::cout << "Indice Count: " << std::to_string(ind.IndicesCount2) << std::endl;
					std::cout << "Indice Offset: " << std::to_string(ind.IndicesOffset4) << std::endl;
                    std::cout << "Indice Block Size: " << std::to_string(ind.IndicesCount2 * 3 * 2) << std::endl;
                    std::cout << "_______________________________________________________________________________" << std::endl;
                }
                
            }

            Object1 obj;
			obj.objectsVerts = vertices1;
			obj.objectsIndices = indices1;
			obj.VertexSize = vert.entrySize;
			obj.rawIndexBlock = indiceData;
			obj.rawVertBlock = vertexData;
			MODEL.objects.push_back(obj);

			std::cout << "Loaded Object with " << std::to_string(vertices1.size()) << " vertices and " << std::to_string(indices1.size() / 3) << " faces." << std::endl;
            
        }
            
        
    };

    public:

    Model_Chunk() : Chunk() {};
    Model_Chunk(const std::vector<unsigned char>& rawFile) : Chunk(rawFile) {};
    Model_Chunk(const std::vector<unsigned char>& rawVDAT, const std::vector<unsigned char>& rawVGPU, bool& IsBig, std::string& name, ChunkInfo& Info)
        : Chunk(rawVDAT, rawVGPU, IsBig, name, Info)
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











/*
void LoadModel(std::vector<unsigned char> VDAT, std::vector<unsigned char> VGPU, bool BigEndian)
    {
        if (ExtractAll) return; //Skip loading model if we are extracting all files (Saves time and atm this isnt stable enough to always run)
        MODEL.RG = GetRendergraph(VDAT, BigEndian);

        AddOffsetToVisit(MODEL.RG.ModelInfoOffset);

        int currentOffset;
        int LocalOffset;

        std::vector<ModelVertDef> verts;
        std::vector<ModelIndicesDef> indices;
        std::vector<unsigned char> rawVertexBlock;
        std::vector<unsigned char> rawIndiceBlock;


        //Main Model Traversal Loop
        while (offsetCount > 0) {
            currentOffset = offsetsToVisit[--offsetCount];

            ModelInfo mi;
            if(currentOffset + sizeof(ModelInfo) > VDAT.size()) {
                std::cout << "Current offset exceeds VDAT size, stopping traversal." << std::endl;
                break;
            }
            memcpy(&mi, &VDAT.data()[currentOffset], sizeof(ModelInfo));

            if (BigEndian) {
                mi.NextOffset = _byteswap_ulong(mi.NextOffset);
                mi.AdditonalOffset = _byteswap_ulong(mi.AdditonalOffset);
                mi.LastOffset = _byteswap_ulong(mi.LastOffset);
            }

            LocalOffset = currentOffset + 16;
            if (mi.Type == 2) {
                ModelVertDef vert;
                memcpy(&vert, &VDAT.data()[currentOffset], sizeof(ModelVertDef));
                if (BigEndian) {
                    vert.info.NextOffset = _byteswap_ulong(vert.info.NextOffset);
                    vert.info.AdditonalOffset = _byteswap_ulong(vert.info.AdditonalOffset);
                    vert.info.LastOffset = _byteswap_ulong(vert.info.LastOffset);
                    vert.vertexCount = _byteswap_ulong(vert.vertexCount);
                    vert.dataOffset = _byteswap_ulong(vert.dataOffset);
                    vert.vertexOffset = _byteswap_ulong(vert.vertexOffset);
                    vert.vertTableLength = _byteswap_ulong(vert.vertTableLength);
                    vert.entrySize = _byteswap_ushort(vert.entrySize);
                }
                verts.push_back(vert);
            }
            else if (mi.Type == 6 || mi.Type == 7) {
                ModelIndicesDef indice;
                memcpy(&indice, &VDAT.data()[currentOffset], sizeof(ModelIndicesDef));
                if (BigEndian) {
                    indice.info.NextOffset = _byteswap_ulong(indice.info.NextOffset);
                    indice.info.AdditonalOffset = _byteswap_ulong(indice.info.AdditonalOffset);
                    indice.info.LastOffset = _byteswap_ulong(indice.info.LastOffset);
                    indice.IndicesSize = _byteswap_ulong(indice.IndicesSize);
                    indice.IndicesCount = _byteswap_ulong(indice.IndicesCount);
                    //indice.IndicesCount2 = _byteswap_ulong(indice.IndicesCount2);
                    indice.IndicesOffset = _byteswap_ulong(indice.IndicesOffset);
                    indice.IndicesOffset2 = _byteswap_ulong(indice.IndicesOffset2);
                    indice.IndicesOffset3 = _byteswap_ulong(indice.IndicesOffset3);
                }
                indices.push_back(indice);
            }

            // Queue both Next and Additional offsets
            AddOffsetToVisit(mi.NextOffset);
            AddOffsetToVisit(mi.AdditonalOffset);
        }

        //for each vert definition, export the vertices
        int y = 0;
        for (const auto& vert : verts) {
            ModelBlock mb;
            mb.VertDef = vert;

            int CurrentOffset = vert.info.NextOffset;
            ModelInfo mi_check;
            while (1) {
                memcpy(&mi_check, &VDAT.data()[CurrentOffset], sizeof(ModelInfo));
                if (BigEndian) {
                    mi_check.NextOffset = _byteswap_ulong(mi_check.NextOffset);
                    mi_check.AdditonalOffset = _byteswap_ulong(mi_check.AdditonalOffset);
                    mi_check.LastOffset = _byteswap_ulong(mi_check.LastOffset);
                }
                if (mi_check.Type == 6) { //Found indice definition
                    break;
                }
                else if (mi_check.NextOffset == 0) { //No more next offsets to check, stop looking
                    break;
                }
                else { //Keep looking
                    CurrentOffset = mi_check.NextOffset;
                }
            }

            ModelIndicesDef mi3;
            if (mi_check.Type == 6) {
                memcpy(&mi3, &VDAT.data()[CurrentOffset], sizeof(ModelIndicesDef));
                mb.IndiceDef = mi3;
            }


            if(mb.IndiceDef.info.Type != 6) {
                std::cout << "Warning: Expected indice definition type 6" << std::to_string(mb.IndiceDef.info.Type) << ". This may cause issues." << std::endl;
            }



            if (BigEndian) {
                mb.IndiceDef.info.NextOffset = _byteswap_ulong(mb.IndiceDef.info.NextOffset);
                mb.IndiceDef.info.AdditonalOffset = _byteswap_ulong(mb.IndiceDef.info.AdditonalOffset);
                mb.IndiceDef.info.LastOffset = _byteswap_ulong(mb.IndiceDef.info.LastOffset);
                mb.IndiceDef.IndicesSize = _byteswap_ulong(mb.IndiceDef.IndicesSize);
                mb.IndiceDef.IndicesCount = _byteswap_ulong(mb.IndiceDef.IndicesCount);
                //mb.IndiceDef.IndicesCount2 = _byteswap_ulong(mb.IndiceDef.IndicesCount2);
                mb.IndiceDef.IndicesOffset = _byteswap_ulong(mb.IndiceDef.IndicesOffset);
                mb.IndiceDef.IndicesOffset2 = _byteswap_ulong(mb.IndiceDef.IndicesOffset2);
                mb.IndiceDef.IndicesOffset3 = _byteswap_ulong(mb.IndiceDef.IndicesOffset3);
                mb.IndiceDef.IndicesOffset4 = _byteswap_ulong(mb.IndiceDef.IndicesOffset4);
            }

            std::vector<Vertex1> vertices1;
            vertices1.resize(mb.VertDef.vertexCount);


            std::vector<unsigned char> vertexData;
            vertexData.resize(mb.VertDef.vertexCount * mb.VertDef.entrySize);
            memcpy(vertexData.data(), &VGPU.data()[mb.VertDef.vertexOffset], mb.VertDef.vertexCount * mb.VertDef.entrySize);

            rawVertexBlock = vertexData; //Save raw vertex block for potential future use


            for (uint32_t i = 0; i < mb.VertDef.vertexCount; i++) {
                Vertex1 v;

                std::vector<unsigned char> VertBlockData;
                VertBlockData.resize(mb.VertDef.entrySize);

                memcpy(VertBlockData.data(), &vertexData[i * mb.VertDef.entrySize], mb.VertDef.entrySize);

                VertexBlock block = ConstructVertexBlockFromSize(mb.VertDef.entrySize, BigEndian, VertBlockData);
                v.position = block.position;
                v.normal = block.normal;
                v.texCoord = block.texCoord;
                vertices1[i] = v;
            }

            //std::cout << "Vert offset: " << std::to_string(mb.VertDef.vertexOffset) << std::endl;
            //std::cout << "Vertex count: " << std::to_string(mb.VertDef.vertexCount) << std::endl;
            //std::cout << "Entry size: " << std::to_string(mb.VertDef.entrySize) << std::endl;
            //std::cout << "Model Index: " << std::to_string(y) << std::endl;

            int IndicesOffset = 0;
            int IndicesCount = mb.IndiceDef.IndicesCount;

            if (mb.IndiceDef.IndicesOffset != 0) {
                std::cout << "Indice block 1 used." << std::endl;
            }
            if (mb.IndiceDef.IndicesOffset2 != 0) {
                std::cout << "Indice block 2 used." << std::endl;
            }
            if (mb.IndiceDef.IndicesOffset3 != 0) {
                std::cout << "Indice block 3 used." << std::endl;
            }
            if (mb.IndiceDef.IndicesOffset4 != 0) {
                std::cout << "Indice block 4 used." << std::endl;
            }
            std::cout << "Face Type: " << std::to_string(mb.IndiceDef.unk1) << std::endl;
            std::cout << "Flag: " << std::to_string(mi_check.Flag2) << std::endl;

            std::cout << "Indice count 1: " << std::to_string(mb.IndiceDef.IndicesCount) << std::endl;
            std::cout << "Indice count 2: " << std::to_string(mb.IndiceDef.IndicesCount2) << std::endl;
            std::cout << "Indice count 3: " << std::to_string(mb.IndiceDef.IndicesCount3) << std::endl;

            std::cout << "IndicesOffset1: " << std::to_string(mb.IndiceDef.IndicesOffset) << std::endl;
            std::cout << "IndicesOffset2: " << std::to_string(mb.IndiceDef.IndicesOffset2) << std::endl;
            std::cout << "IndicesOffset3: " << std::to_string(mb.IndiceDef.IndicesOffset3) << std::endl;
            std::cout << "IndicesOffset4: " << std::to_string(mb.IndiceDef.IndicesOffset4) << std::endl;

            std::cout << "Vertex Offset: " << std::to_string(mb.VertDef.vertexOffset) << std::endl;
            std::cout << "Vertex Count: " << std::to_string(mb.VertDef.vertexCount) << std::endl;
            std::cout << "Vertex Entry Size: " << std::to_string(mb.VertDef.entrySize) << std::endl;

            return;



            std::cout << "IndicesOffset: " << std::to_string(IndicesOffset) << std::endl;
            std::cout << "IndicesCount: " << std::to_string(IndicesCount) << std::endl;

            std::cout << "IndiceDef Hex: ";
            unsigned char* p = (unsigned char*)&mb.IndiceDef;
            for (size_t i = 0; i < sizeof(ModelIndicesDef); i++) {
                printf("%02X ", p[i]);
            }

            std::vector<GLuint> indices1;
            int CurrentIndiceOffset = IndicesOffset;
            bool use32BitIndices = mb.VertDef.vertexCount > std::numeric_limits<uint16_t>::max();
            rawIndiceBlock.resize(IndicesCount * (use32BitIndices ? 4 : 2)); //Each indice is 2 or 4 bytes, and there are 3 indices per triangle
            memcpy(rawIndiceBlock.data(), &VGPU.data()[IndicesOffset], IndicesCount * (use32BitIndices ? 4 : 2));

            if (!use32BitIndices) {
                std::vector<uint16_t> tempIndices;
                tempIndices.resize(IndicesCount);
                memcpy(tempIndices.data(), &VGPU.data()[IndicesOffset], (IndicesCount) * sizeof(uint16_t));
                if (BigEndian) {
                    for (auto& index : tempIndices) {
                        index = _byteswap_ushort(index);
                    }
                }
                for (const auto& index : tempIndices) {
                    indices1.push_back(static_cast<GLuint>(index));
                }
            }
            else {
                std::vector<uint32_t> tempIndices;
                tempIndices.resize(IndicesCount);
                memcpy(tempIndices.data(), &VGPU.data()[IndicesOffset], (IndicesCount) * sizeof(uint32_t));
                if (BigEndian) {
                    for (auto& index : tempIndices) {
                        index = _byteswap_ulong(index);
                    }
                }
                for (const auto& index : tempIndices) {
                    indices1.push_back(static_cast<GLuint>(index));
                }
            }

            std::cout << "Extracted indices count: " << indices1.size() << std::endl;
            if (!indices1.empty()) {
                std::cout << "First few indices: ";
                for (size_t i = 0; i < std::min<size_t>(indices1.size(), 9); ++i)
                    std::cout << indices1[i] << " ";
                std::cout << std::endl;
            }

            //old indice extraction method, kept for reference
            if (indices1.empty()) {
                std::cout << "No indices extracted, attempting old extraction method." << std::endl;
                int IndicesOffset = 0;
                int IndicesCount = mb.IndiceDef.IndicesCount;
                if (mb.IndiceDef.IndicesOffset != 0) {
                    IndicesOffset = mb.IndiceDef.IndicesOffset;
                }
                else {
                    if (mb.IndiceDef.IndicesOffset2 != 0) {
                        if (mb.IndiceDef.IndicesOffset3 != 0) {
                            if (mb.IndiceDef.IndicesOffset3 < mb.IndiceDef.IndicesOffset2) {
                                 IndicesOffset = mb.IndiceDef.IndicesOffset3;

                            }
                            else {
                                 IndicesOffset = mb.IndiceDef.IndicesOffset2;
                            }
                        }
                        else {
                             IndicesOffset = mb.IndiceDef.IndicesOffset2;
                        }
                    }
                    else if (mb.IndiceDef.IndicesOffset3 != 0) {
                          IndicesOffset = mb.IndiceDef.IndicesOffset3;
                    }
                }

                while (true) {
                    if (use32BitIndices) {
                        if (CurrentIndiceOffset + 11 >= VGPU.size()) {
                            std::cout << "Reached end of VGPU data or next vertex offset, stopping indice extraction." << std::endl;
                            break;
                        }
                        else if (verts.size() - 1 >= y + 1) {
                            if (CurrentIndiceOffset + 11 >= GetNextVertexBlock(verts[y], verts).vertexOffset) {
                                std::cout << "Reached next vertex offset, stopping indice extraction." << std::endl;
                                break;
                            }
                        }

                        int32_t index1, index2, index3;
                        memcpy(&index1, &VGPU.data()[CurrentIndiceOffset], sizeof(int32_t));
                        memcpy(&index2, &VGPU.data()[CurrentIndiceOffset + 4], sizeof(int32_t));
                        memcpy(&index3, &VGPU.data()[CurrentIndiceOffset + 8], sizeof(int32_t));
                        if (BigEndian) {
                            index1 = _byteswap_ulong(index1);
                            index2 = _byteswap_ulong(index2);
                            index3 = _byteswap_ulong(index3);
                        }

                        if (index1 == 0 && index2 == 0 && index3 == 0) {
                            std::cout << "Reached 0,0,0 indice, stopping indice extraction." << std::endl;
                            break;
                        }

                        indices1.push_back(static_cast<GLuint>(index1));
                        indices1.push_back(static_cast<GLuint>(index2));
                        indices1.push_back(static_cast<GLuint>(index3));
                        CurrentIndiceOffset += 12;
                    }
                    else {
                        if (CurrentIndiceOffset + 5 >= VGPU.size()) {
                            std::cout << "Reached end of VGPU data or next vertex offset, stopping indice extraction." << std::endl;
                            break;
                        }
                        else if (verts.size() - 1 >= y + 1) {
                            if (CurrentIndiceOffset + 5 >= GetNextVertexBlock(verts[y], verts).vertexOffset) {
                                std::cout << "Reached next vertex offset, stopping indice extraction." << std::endl;
                                break;
                            }
                        }

                        uint16_t index1, index2, index3;
                        memcpy(&index1, &VGPU.data()[CurrentIndiceOffset], sizeof(uint16_t));
                        memcpy(&index2, &VGPU.data()[CurrentIndiceOffset + 2], sizeof(uint16_t));
                        memcpy(&index3, &VGPU.data()[CurrentIndiceOffset + 4], sizeof(uint16_t));
                        if (BigEndian) {
                            index1 = _byteswap_ushort(index1);
                            index2 = _byteswap_ushort(index2);
                            index3 = _byteswap_ushort(index3);
                        }

                        if (index1 == 0 && index2 == 0 && index3 == 0) {
                            std::cout << "Reached 0,0,0 indice, stopping indice extraction." << std::endl;
                            break;
                        }

                        indices1.push_back(index1);
                        indices1.push_back(index2);
                        indices1.push_back(index3);
                        CurrentIndiceOffset += 6;
                    }
                }
            }
Object1 obj;
obj.objectsVerts = vertices1;
obj.objectsIndices = indices1;
obj.VertexSize = mb.VertDef.entrySize;
obj.rawVertBlock = rawVertexBlock;
obj.rawIndexBlock = rawIndiceBlock;
MODEL.objects.push_back(obj);

y++;
        }
    };

*/