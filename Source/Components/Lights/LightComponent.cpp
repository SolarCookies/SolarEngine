#include "LightComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void LightComponent::BeginPlay()
{

	lightModel = glm::translate(lightModel, lightPos);

	lightMesh.model.SetMaterialParameter("model", lightModel);
	lightMesh.model.SetMaterialParameter("lightColor", lightColor);

	lightMesh.model.SetMaterialParameter("tex0", SunTexture);
}

void LightComponent::EndPlay()
{
	SunTexture.Delete();
	lightMesh.model.Destroy();
}

void LightComponent::Tick(float deltaTime, World* World)
{
}

void LightComponent::Render(VinceWindow* window, Camera* Cam)
{
	lightMesh.model.SetMaterialParameter("lightColor", lightColor);
	lightMesh.model.SetMaterialParameter("lightPos", lightPos);


	// Compute billboard model matrix for the sun
	glm::mat4 lightModel = glm::mat4(1.0f);
	lightModel = glm::translate(lightModel, lightPos);

	// Extract rotation from the camera view matrix and invert it
	glm::mat4 view = Cam->getViewMatrix();
	glm::mat4 rotation = glm::mat4(glm::mat3(view));
	rotation = glm::transpose(rotation);

	lightModel *= rotation;

	// Scale to light model to 0.02f, 0.01f, 0.02f
	lightModel = glm::scale(lightModel, glm::vec3(0.02f, 0.01f, 0.02f));

	// Set the model matrix for the light mesh
	lightMesh.model.SetMaterialParameter("model", lightModel);

	Cam->Matrix(lightMesh.model.shaderProgram, "camMatrix");

	SunTexture.Bind();

	Draw();
}

void LightComponent::Draw()
{
	lightMesh.model.Draw();
}
