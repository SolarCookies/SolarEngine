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
		if (ColorTexture->IsValid) {
			if (OBJ.model.GetNumVertices() != 0) {
				OBJ.model.SetMaterialParameter("tex0", *ColorTexture);
			}
		}
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
	Cam->Matrix(*OBJ.model.material->shaderProgram.get(), "camMatrix");

	if (ColorTexture != nullptr)
	{
		glActiveTexture(GL_TEXTURE0);
		ColorTexture->Bind();
	}
	Draw(Cam->ShadowPerspective,window,Cam);
}

void StaticMeshComponent::Draw(bool shadow, VinceWindow* window, Camera* Cam)
{
	
	OBJ.model.Draw(false,0,shadow,window,Cam);
}
