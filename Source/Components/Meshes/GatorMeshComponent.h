#pragma once
#include "DynamicMeshComponent.h"
#include "../../Shaders/Texture.h"

class VinceWorldLoader; // Forward declaration

class GatorMeshComponent : public DynamicMeshComponent
{
public:
	GatorMeshComponent(const std::vector<unsigned char>& MeshFile, std::string NameOfFile,VinceWorldLoader* Owner)
		: DynamicMeshComponent("Color")
	{
		// Constructor implementation can go here if needed
		Mesh = MeshFile;
		name = "GatorMeshComponent";
		GatorName = NameOfFile;
		Loader = Owner;
	}

	void Construct() override;

    std::string GuessColorTexture() {
        if (AssetNames.empty()) return "";
        for (const auto& name : AssetNames) {
            if (name.find(".dds") != std::string::npos) {
				return name; // Return the first DDS file found
            }
        }
	}

	void LoadGuessedTexture();

    void LoadGatorMesh(const std::vector<unsigned char>& MeshFile) {
        Mesh = MeshFile;
        Construct();
	}

    void RenderDetails() override {
        ImGui::Text(GatorName.c_str());
        DynamicMeshComponent::RenderDetails();
	}


private:
	std::vector<unsigned char> Mesh;
	std::string GatorName;
    std::vector<std::string> AssetNames; //List of names at the end of the file
	VinceWorldLoader* Loader;
};
