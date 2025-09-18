#include "GatorMeshComponent.h"
#include "../../World/LoadVinceWorld.h"

void GatorMeshComponent::Construct()
{
    std::vector<DynamicVertex> Vertices;
    std::vector<GLuint> Indices;

    if (Mesh.size() < 4) {
        std::cerr << "GatorMeshComponent: Mesh data is too small." << std::endl;
        return;
    }

    // Check magic
    if (std::memcmp(Mesh.data(), "GATR", 4) != 0) {
        std::cerr << "GatorMeshComponent: Invalid mesh magic." << std::endl;
        return;
    }

    // Parse header as in Python
    const size_t offset_header1 = 4;
    const size_t offset_header2 = offset_header1 + 10 * sizeof(uint32_t);
    const size_t offset_header3 = offset_header2 + 2 * sizeof(uint16_t);

    uint32_t header1[10];
    std::memcpy(header1, Mesh.data() + offset_header1, 10 * sizeof(uint32_t));
    uint16_t header2[2];
    std::memcpy(header2, Mesh.data() + offset_header2, 2 * sizeof(uint16_t));
    uint32_t header3[9];
    std::memcpy(header3, Mesh.data() + offset_header3, 9 * sizeof(uint32_t));

    uint32_t vertCount = header1[3];
    uint32_t tstrip_count = header1[5];
    uint32_t startOfVerts = header3[0];
    uint32_t startOfTris = header3[1];
    uint32_t tstrip_table = header3[2];

    //strings are at offset 80
    uint32_t stringOffset = header3[8];
    Log("String Offset: " + std::to_string(stringOffset), EType::Warning);

    uint32_t NumberOfBytesToRead = Mesh.size() - stringOffset;
    Log("Number of bytes to read: " + std::to_string(NumberOfBytesToRead), EType::Warning);

    std::string CurrentString;
    for (uint32_t i = 0; i < NumberOfBytesToRead; ++i) {
        if (Mesh[stringOffset + i] == 0) {
            if (CurrentString.empty()) continue; // Skip empty strings
            AssetNames.push_back(CurrentString);
            CurrentString.clear();
        }
        else {
            CurrentString += static_cast<char>(Mesh[stringOffset + i]);
        }
    }



    // Read vertices
    if (Mesh.size() < startOfVerts + vertCount * 0x40) {
        std::cerr << "GatorMeshComponent: Mesh data is too small for vertices." << std::endl;
        return;
    }
    for (uint32_t i = 0; i < vertCount; ++i) {
        float vertData[16];
        std::memcpy(vertData, Mesh.data() + startOfVerts + i * 0x40, 0x40);

        float x = -vertData[0]; // Flip X
        float y = vertData[1];
        float z = vertData[2];
        float nx = -vertData[5]; // Flip normal X
        float ny = vertData[6];
        float nz = vertData[7];
        float u = vertData[12] * 6.0f;
        float v = (vertData[13] * 6.0f * -1.0f) + 1.0f;

        Vertices.push_back({ {x, y, z}, {1.0f, 1.0f, 1.0f}, {u, v}, {nx, ny, nz} });
    }

    // Read triangle strip counts
    std::vector<uint32_t> strip_counts;
    size_t tstrip_offset = tstrip_table;
    for (uint32_t i = 0; i < tstrip_count; ++i) {
        if (Mesh.size() < tstrip_offset + 4) {
            std::cerr << "GatorMeshComponent: Mesh data is too small for triangle strip count." << std::endl;
        }
        uint32_t count;
        std::memcpy(&count, Mesh.data() + tstrip_offset, 4);
        strip_counts.push_back(count);
        tstrip_offset += 48;
    }

    // Read triangle indices
    size_t tris_offset = startOfTris;
    for (uint32_t face_count : strip_counts) {
        if (Mesh.size() < tris_offset + face_count * 2) {
            std::cerr << "GatorMeshComponent: Mesh data is too small for triangle indices." << std::endl;
        }
        std::vector<uint16_t> strip(face_count);
        std::memcpy(strip.data(), Mesh.data() + tris_offset, face_count * 2);
        tris_offset += face_count * 2;

        for (uint32_t face = 0; face + 2 < face_count; ++face) {
            uint16_t f1 = strip[face + 0];
            uint16_t f2 = strip[face + 1];
            uint16_t f3 = strip[face + 2];
            if (face & 1)
                Indices.insert(Indices.end(), { f1, f2, f3 });
            else
                Indices.insert(Indices.end(), { f2, f1, f3 });
        }
    }

	LoadGuessedTexture();

    if(ColorTexture.ID != 0) {
		shaderName = "LitAlbedo";
    }

    ConstructMesh(Vertices, Indices);
    DynamicMeshComponent::Construct();
}

void GatorMeshComponent::LoadGuessedTexture()
{
    if (Loader) {
		std::string NameOfTexture = GuessColorTexture();
		std::string NameOfTextureLower = NameOfTexture;
		std::transform(NameOfTextureLower.begin(), NameOfTextureLower.end(), NameOfTextureLower.begin(), ::tolower);
        for(const HotFileInfo& file : Loader->TextureData) {
            if (file.Name == NameOfTextureLower) {
                ColorTexture = Texture(file.Data);
				//std::cout << "GatorMeshComponent: Loaded texture " << NameOfTextureLower << std::endl;
                return;
            }
            else {
				//std::cout << "Not a Match : " << file.Name << " vs " << NameOfTextureLower << std::endl;
            }
		}
		std::cout << "No Texture found for " << NameOfTextureLower << std::endl;
        //std::cout << "Number of Textures in world: " << Loader->TextureData.size() << std::endl;
    }
}
