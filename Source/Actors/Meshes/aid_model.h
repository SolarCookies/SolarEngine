#pragma once
#include "../Actor.h"
#include "../../Components/Meshes/DynamicMeshComponent.h"
#include "../../Temporary/Mesh.h"
#include "../../Windows/Log.hpp"
#include "../../GlobalSettings.h"
#include "../../Temporary/Debug.h"
#include "../../Package/VP/FileSystem/GlobalPackages.h"

class Aid_Model : public Actor
{

public:
	std::string Name = "";
	Aid_Model()
		: Actor()
	{
	}
	virtual ~Aid_Model() {}
	void Tick(float deltaTime, World* world) override
	{
		Actor::Tick(deltaTime, world);


		if (CurrentModelName == "") {
			//clear all models from components
			RemoveAllComponentsByName("DynamicMesh");
		}

		if (Name != CurrentModelName) {
			Name = CurrentModelName;
			Log("Loading Model Into Viewport: " + Name, EType::Success);

			//for all components with DynamicMesh, remove them

			RemoveAllComponentsByName("DynamicMesh");


			int numModels = CurrentModel.size();
			for(int l = 0; l < numModels; l++) {

				auto dynamicMeshComponent = std::make_shared<DynamicMeshComponent>("LitAlbedo");
				Texture* texture1 = LookupTexture(CurrentModel[l].ColorTextureName);
				if(texture1 != nullptr) {
					dynamicMeshComponent->ColorTexture = *texture1;
					
					//Texture* texture2 = LookupTexture(CurrentModel[l].NormalTextureName);
					//if(texture2 != nullptr) {
						//dynamicMeshComponent->NormalTexture = *texture2;
					//}
					//else {
					//	std::cout << "Normal Texture not found: " << CurrentModel[l].NormalTextureName << std::endl;
					//}
				}
				else {
					//std::cout << "Color Texture not found: " << CurrentModel[l].ColorTextureName << std::endl;
				}

				std::vector<DynamicVertex> Vertices;
				std::vector<GLuint> Triangles;

				int i = 0;
				Vertices.resize(CurrentModel[l].objectsVerts.size());
				for (const Vertex1& vertex : CurrentModel[l].objectsVerts) {
					DynamicVertex v;
					v.position.x = (vertex.position.x / 100.0);
					v.position.y = (vertex.position.y / 100.0);
					v.position.z = (vertex.position.z / 100.0);

					v.normal.x = vertex.normal.x;
					v.normal.y = vertex.normal.y;
					v.normal.z = vertex.normal.z;

					v.color.x = 1.0f;
					v.color.y = 1.0f;
					v.color.z = 1.0f;

					v.texCoords.x = vertex.texCoord.u; // 0.0f to 1.0f
					v.texCoords.y = vertex.texCoord.v; // 0.0f to 1.0f

					Vertices[i] = v;
					i++;
				}
				i = 0;

				Triangles.resize(CurrentModel[l].objectsIndices.size());
				for (int y = 0; y < CurrentModel[l].objectsIndices.size(); y++) {
					Triangles[y] = CurrentModel[l].objectsIndices[y];
				}

				dynamicMeshComponent->ConstructMesh(Vertices, Triangles, true);
				AddComponent(std::move(dynamicMeshComponent));
			}

			
		}

	}


	void RenderDetails() override {
		if (ImGui::SliderFloat3("Position", &WorldPosition.x, -100.0f, 100.0f)) {
			UpdateTransform();
		}
		if (ImGui::SliderFloat3("Rotation", &WorldRotation.x, -3.14f, 3.14f)) {
			//    UpdateTransform();
			Log("Rotation doesn't work yet");
		}
		if (ImGui::SliderFloat3("Scale", &WorldScale.x, 0.1f, 10.0f)) {
			UpdateTransform();
		}
		int f = 0;
		for (const Object1& model : CurrentModel) {
			ImGui::Text(std::string("Model Part " + std::to_string(f)).c_str());
			ImGui::Text(("Vertex Count: " + std::to_string(model.objectsVerts.size())).c_str());
			ImGui::Text(("Index Count: " + std::to_string(model.objectsIndices.size())).c_str());
			ImGui::Text(("Vertex Size: " + std::to_string(model.VertexSize)).c_str());
			ImGui::BeginChild(std::string("Model Info " + std::to_string(f)).c_str(), ImVec2(0, 100), true);
			if(ImGui::Button("View Vertex Block")) {
				FileName = "Vertex Block";
				FileViewType = 1;
				FileViewBuffer = model.rawVertBlock;
				GlobalSaveFunction = [this](const std::vector<unsigned char>& data) { };
			}
			if (ImGui::Button("View Indices Block")) {
				FileName = "Indices Block";
				FileViewType = 1;
				FileViewBuffer = model.rawIndexBlock;
				GlobalSaveFunction = [this](const std::vector<unsigned char>& data) {};
			}

			if (ImGui::Button("Export Vertex Block")) {
				FileName = "Vertex Block";
				FileViewType = 1;
				FileViewBuffer = model.rawVertBlock;
				GlobalSaveFunction = [this](const std::vector<unsigned char>& data) {};
			}
			if (ImGui::Button("Export Indices Block")) {
				FileName = "Indices Block";
				FileViewType = 1;
				FileViewBuffer = model.rawIndexBlock;
				GlobalSaveFunction = [this](const std::vector<unsigned char>& data) {};
			}

			//Text that shows the first u and v coordinate of the texture coordinates
			if(model.objectsVerts.size() > 0) {
				ImGui::Text(("First UV Coordinate: " + std::to_string(model.objectsVerts[0].texCoord.u) + ", " + std::to_string(model.objectsVerts[0].texCoord.v)).c_str());
				ImGui::Text(("UV Offset: " + std::to_string(model.objectsVerts[0].extraData.texCoordOffset)).c_str());
			}

			ImGui::EndChild();
			f++;
		}
		
		
	}


protected:

private:

};