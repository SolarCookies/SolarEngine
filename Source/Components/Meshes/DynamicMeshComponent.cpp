#include "DynamicMeshComponent.h"
#include "../../World/World.h"
#include "../Lights/LightComponent.h"

void DynamicMeshComponent::Construct()
{
	ReInitializeModel();
	//add owner transform to the model
	Mat = owner->GetTransform();
	Mat = glm::translate(Mat, Position);
	OBJ->SetMaterialParameter("model", Mat);
}

void DynamicMeshComponent::BeginPlay()
{
}

void DynamicMeshComponent::Tick(float deltaTime, World* World)
{
	if (World == nullptr)
		return;
	LightComponent* lightSource = World->GetLightSource();
	if (lightSource)
	{
		OBJ->SetMaterialParameter("lightColor", lightSource->lightColor);
		OBJ->SetMaterialParameter("lightPos", lightSource->lightPos);
	}
	else {
		OBJ->SetMaterialParameter("lightColor", glm::vec4(0));
		OBJ->SetMaterialParameter("lightPos", glm::vec3(0));
	}

	//update the model matrix with the current transform
	Mat = owner->GetTransform();
	Mat = glm::translate(Mat, Position);
	OBJ->SetMaterialParameter("model", Mat);
}

void DynamicMeshComponent::Render(VinceWindow* window, Camera* Cam)
{
	//OBJ.model.SetMaterialParameter("unlit", unlit ? 1 : 0);
	OBJ->SetMaterialParameter("unlit", 0);

	OBJ->SetMaterialParameter("cameraPos", Cam->Position);
	Cam->Matrix(OBJ->shaderProgram, "camMatrix");

	Draw();
}

void DynamicMeshComponent::Draw()
{
	OBJ->Draw();
}
