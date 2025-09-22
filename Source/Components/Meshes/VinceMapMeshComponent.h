#pragma once
#include "../Component.h"
#include "DynamicMeshComponent.h"
#include "../../Shaders/DebugDraw.h"
#include "../../Utils/ZLibHelpers.h"
#include <random>

struct DynamicStrip {
	int StripIndex = 0;
    std::vector<DynamicVertex> Vertices;
};

class VinceMapMeshComponent : public Component
{
public:
    VinceMapMeshComponent(const std::vector<unsigned char>& MeshFile)
        : Component()
    {
        // Constructor implementation can go here if needed
        Mesh = MeshFile;
        name = "VinceMapMeshComponent";
    }

    vec3 RandomColor(int seed) {
        std::mt19937 generator(seed);
        std::uniform_int_distribution<int> distribution(0, colors.size() - 1);
		return colors[distribution(generator)];
	}

    void Construct() override {
        std::vector<DynamicVertex> AllVertices;
		Strips.clear(); // Clear previous strips

        if (Mesh.size() < 4) {
            std::cerr << "VinceMapMeshComponent: Mesh data is too small." << std::endl;
            return;
        }
        DynamicVertex Lastvertex;
        DynamicStrip Strip;
        // Build all vertices
        for (int i = 0; i < VertexCount; i += 1) {
            DynamicVertex vertex;
            std::vector<unsigned char> xBytes = Vince::CopyBytes(Mesh, i * VertexSpacing, 4);
            std::vector<unsigned char> yBytes = Vince::CopyBytes(Mesh, i * VertexSpacing + 4, 4);
            std::vector<unsigned char> zBytes = Vince::CopyBytes(Mesh, i * VertexSpacing + 8, 4);
            float x = Vince::ConvertBytesToFloat(xBytes, false);
            float y = Vince::ConvertBytesToFloat(yBytes, false);
            float z = Vince::ConvertBytesToFloat(zBytes, false);

            vertex.position = glm::vec3(x, y, z);

            // Color
            std::vector<unsigned char> rBytes = Vince::CopyBytes(Mesh, i * VertexSpacing + 48, 1);
            std::vector<unsigned char> gBytes = Vince::CopyBytes(Mesh, i * VertexSpacing + 49, 1);
            std::vector<unsigned char> bBytes = Vince::CopyBytes(Mesh, i * VertexSpacing + 50, 1);
            float r = static_cast<float>(rBytes[0]) / 255.0f;
            float g = static_cast<float>(gBytes[0]) / 255.0f;
            float b = static_cast<float>(bBytes[0]) / 255.0f;
            vertex.color = glm::vec3(r, g, b);

            if (Lastvertex.position == glm::vec3(x, y, z)) {
                Strips.push_back(Strip); // Save the current strip
                Strip = DynamicStrip(); // Start a new strip
                Strip.StripIndex = Strips.size(); // Set the index of the new strip
				Log("Strip " + std::to_string(Strip.StripIndex) + " started with vertex: " + std::to_string(i), EType::Success);
            }
            else {
                AllVertices.push_back(vertex);
                Lastvertex = vertex;
                Strip.Vertices.push_back(vertex);
            }
            if (i == VertexCount - 1) {
                Strips.push_back(Strip); // Save the last strip
            }
        }
		std::vector<unsigned int> Indices;

        DynamicMesh = std::make_shared<DynamicMeshComponent>("Color");
		DynamicMesh->isTriangleStrip = true;
		DynamicMesh->SetMeshSilently(AllVertices, Indices);
		//owner->AddComponent(DynamicMesh);
    }

    void LoadVinceMapMesh(const std::vector<unsigned char>& MeshFile) {
        Mesh = MeshFile;
        Construct();
    }

    void RenderDetails() override {
        //List vertices and indices for debugging purposes
        if (ImGui::SliderInt("Vertex Count", &VertexCount, 0, 1000)) {
            Construct();
            //ReInitializeModel();
        }
        if (ImGui::SliderInt("Vertex Spacing", &VertexSpacing, 0, 200)) {
            Construct();
        }
        if (ImGui::SliderInt("Triangle Count", &TriangleCount, 0, 100)) {
            Construct();
        }
        if (ImGui::InputInt("Vertex Count Int", &VertexCount)) {
            Construct();
        }
        if (ImGui::InputInt("Vertex Spacing Int", &VertexSpacing)) {
            Construct();
        }
        if (ImGui::InputInt("Triangle Count Int", &TriangleCount)) {
            Construct();
        }
    }

    void Render(VinceWindow* window, Camera* Cam) override {
        
	}

private:
    std::vector<DynamicStrip> Strips;
    std::vector<unsigned char> Mesh;
	int VertexCount = 120094;//120094
	int VertexSpacing = 76;
	int TriangleCount = 0;
    std::vector<vec3> colors = {
        {1.0f, 0.0f, 0.0f}, // Red
        {0.0f, 1.0f, 0.0f}, // Green
        {0.0f, 0.0f, 1.0f}, // Blue
        {1.0f, 1.0f, 0.0f}, // Yellow
        {1.0f, 0.0f, 1.0f}, // Magenta
        {0.0f, 1.0f, 1.0f}, // Cyan
        {0.5f, 0.5f, 0.5f}, // Gray
        {1.0f, 0.5f, 0.5f}, // Light Red
        {0.5f, 1.0f, 0.5f}, // Light Green
        {0.5f, 0.5f, 1.0f}, // Light Blue
        {1.0f, 1.0f, 1.0f}, // White
        {0.8f, 0.8f, 0.8f}, // Light Gray
		{1.0f, 0.75f, 0},   // Orange
        {0.75f, 0.5f, 0.25f}, // Brown
		{0.5f, 0.25f, 0.75f},  // Purple
		{0.25f, 0.75f, 0.5f},   // Teal
		{0.75f, 0.75f, 0.25f}  // Olive
    };
	std::shared_ptr<DynamicMeshComponent> DynamicMesh = nullptr;
};
