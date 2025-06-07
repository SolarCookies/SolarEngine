#include "StaticMeshComponent.h"
#include "../Lights/LightComponent.h"
#include "../../World/World.h"

void StaticMeshComponent::Construct()
{
	//add owner transform to the model
	Mat = owner->GetTransform();
	Mat = glm::translate(Mat, Position);
	OBJ.model.SetMaterialParameter("model", Mat);

	if (ColorTexture != nullptr)
	{
		OBJ.model.SetMaterialParameter("tex0", *ColorTexture);
	}
	if (NormalTexture != nullptr)
	{
		OBJ.model.SetMaterialParameter("tex1", *NormalTexture);
	}
}

void StaticMeshComponent::BeginPlay()
{
	
}

void StaticMeshComponent::Tick(float deltaTime, World* World)
{
	if(World == nullptr)
		return;
	LightComponent* lightSource = World->GetLightSource();
	if (lightSource)
	{
		OBJ.model.SetMaterialParameter("lightColor", lightSource->lightColor);
		OBJ.model.SetMaterialParameter("lightPos", lightSource->lightPos);
	}
	else {
		OBJ.model.SetMaterialParameter("lightColor", glm::vec4(0));
		OBJ.model.SetMaterialParameter("lightPos", glm::vec3(0));
	}

	//update the model matrix with the current transform
	Mat = owner->GetTransform();
	Mat = glm::translate(Mat, Position);
	OBJ.model.SetMaterialParameter("model", Mat);
}

void StaticMeshComponent::Render(VinceWindow* window, Camera* Cam)
{
	//OBJ.model.SetMaterialParameter("unlit", unlit ? 1 : 0);
	OBJ.model.SetMaterialParameter("unlit", 0);

	OBJ.model.SetMaterialParameter("cameraPos", Cam->Position);
	Cam->Matrix(OBJ.model.shaderProgram, "camMatrix");

	if (ColorTexture != nullptr)
	{
		glActiveTexture(GL_TEXTURE0);
		ColorTexture->Bind();
	}
	if (NormalTexture != nullptr)
	{
		glActiveTexture(GL_TEXTURE1);
		NormalTexture->Bind();
	}

	Draw();
}

void StaticMeshComponent::Draw()
{
	OBJ.model.Draw();
}
