#pragma once
#include "../Component.h"
#include "../../Temporary/Mesh.h"

class LightComponent : public Component
{
public:
	LightComponent()
		: lightMesh("Models/sun.obj", "light"), // Provide valid constructor args
		SunTexture("Models/sun.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE)
	{
		name = "LightComponent";
	}
	~LightComponent() override = default;
	void BeginPlay() override;
	void EndPlay() override;
	void Tick(float deltaTime, World* World) override;
	void Render(VinceWindow* window, Camera* Cam) override;
	void Draw() override;
	void SetColor(const glm::vec4& color) { lightColor = color; }
	glm::vec4 GetColor() const { return lightColor; }
	void SetPosition(const glm::vec3& position) { lightPos = position; }
	glm::vec3 GetPosition() const { return lightPos; }

public:
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::mat4 lightModel = glm::mat4(1.0f);

private:
	Mesh lightMesh;
	Texture SunTexture;
};
