#pragma once
#include "../Component.h"
#include "../../Temporary/Model.h"
#include "../Component.h"


// Forward declaration
class World;

struct DynamicVertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoords;
	glm::vec3 normal;
};



class DynamicMeshComponent : public Component
{
public:
	DynamicMeshComponent(std::string ShaderName)
	{
		this->name = "DynamicMesh";
		this->shaderName = ShaderName;
	}
	virtual ~DynamicMeshComponent() = default;

	void Construct() override;
	void BeginPlay() override;
	void Tick(float deltaTime, World* World) override;
	void Render(VinceWindow* window, Camera* Cam) override;
	void Draw() override;

	void ReInitializeModel()
	{
		// Convert DynamicVertex to GLfloat vector
		std::vector<GLfloat> convertedVertices;
		for (const auto& vertex : Vertices)
		{
			convertedVertices.push_back(vertex.position.x);
			convertedVertices.push_back(vertex.position.y);
			convertedVertices.push_back(vertex.position.z);

			convertedVertices.push_back(vertex.color.r);
			convertedVertices.push_back(vertex.color.g);
			convertedVertices.push_back(vertex.color.b);

			convertedVertices.push_back(vertex.texCoords.x);
			convertedVertices.push_back(vertex.texCoords.y);

			convertedVertices.push_back(vertex.normal.x);
			convertedVertices.push_back(vertex.normal.y);
			convertedVertices.push_back(vertex.normal.z);
		}
		
		// Create a new Model instance with the converted vertices and triangles
		OBJ = std::make_unique<Model>(std::string("Assets/Shaders/" + std::string(shaderName) + ".vert").c_str(),
			std::string("Assets/Shaders/" + std::string(shaderName) + ".frag").c_str(),
			convertedVertices,
			Triangles
		);
	}

	void AddTriangle(GLuint v1, GLuint v2, GLuint v3, bool isDirty)
	{
		Triangles.push_back(v1);
		Triangles.push_back(v2);
		Triangles.push_back(v3);
		if(isDirty)
			ReInitializeModel(); //Only reinitialize if the model is dirty Aka this is the last triangle to add
	}

	void AddVertex(const DynamicVertex& vertex, bool isDirty)
	{
		Vertices.push_back(vertex);
		if(isDirty)
			ReInitializeModel(); //Only reinitialize if the model is dirty Aka this is the last vertex to add
	}

	void ConstructMesh(std::vector<DynamicVertex> Vertices, std::vector<GLuint> Triangles, bool isDirty = true)
	{
		this->Vertices = Vertices;
		this->Triangles = Triangles;
		ReInitializeModel(); //Only reinitialize if the model is dirty Aka this is the last triangle to add
	}

	glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 Mat = glm::mat4(1.0f);
	//Texture* ColorTexture;
	//Texture* NormalTexture;

protected:
	//Model* OBJ;
	std::unique_ptr<Model> OBJ; // Temporary model for reinitialization
	std::string shaderName;
	std::vector <DynamicVertex> Vertices;
	std::vector <GLuint> Triangles; // Aka Indices
	// Vertices coordinates
	std::vector<GLfloat> ErrorVertices =
	{ //     COORDINATES     /        COLORS          /    TexCoord   /        NORMALS       //
		0.0f, 0.0f, 0.0f,     0.0f, 0.0f, 0.0f, 	 0.0f, 0.0f,      0.0f, 0.0f, 0.0f, // Bottom side
		0.0f, 0.0f, 0.0f,     0.0f, 0.0f, 0.0f,		 0.0f, 0.0f,      0.0f, 0.0f, 0.0f, // Bottom side
		0.0f, 0.0f, 0.0f,     0.0f, 0.0f, 0.0f,		 0.0f, 0.0f,      0.0f, 0.0f, 0.0f, // Bottom side
	};

	// Indices for vertices order
	std::vector<GLuint> ErrorIndices =
	{
		0, 1, 2, // Bottom side
	};
};