#pragma once
#include "../Component.h"
#include "../../Temporary/Mesh.h"

// Forward declaration
class World;

class StaticMeshComponent : public Component
{
public:
	StaticMeshComponent(std::string OBJPath, std::string ShaderName) : OBJ(OBJPath.c_str(), ShaderName.c_str()) {
		this->name = "StaticMesh";
		this->MeshPath = OBJPath;
		this->shaderName = ShaderName;
	}
	virtual ~StaticMeshComponent() = default;

	void Construct() override;
	void BeginPlay() override;
	void Tick(float deltaTime, World* World) override;
	void Render(VinceWindow* window, Camera* Cam) override;
	void Draw() override;

	void SetStaticMesh(const std::string& meshName) {
	}

	void RenderDetails() {
		ImGui::Text("Number of Vertices: %d", OBJ.model.GetNumVertices());
		ImGui::Text("Number of Triangles: %d", OBJ.model.GetNumIndices() / 3);
		ImGui::Text("Shader Name: %s", shaderName.c_str());
	}

	//template <typename C>
	//void SetMaterialParameter(const std::string& parameterName, const C Value)
	//{
	//	OBJ.model.SetMaterialParameter(parameterName, Value);
	//}

	glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 Mat = glm::mat4(1.0f);
	Texture* ColorTexture;
	Texture* NormalTexture;

protected:
	Mesh OBJ;
	std::string MeshPath = "";
	std::string shaderName;
	
};
