//This file is a extension of Main in order to debug new features.
#pragma once
#include "../World/Camera.h"
#include "../World/World.h"
#include "../Shaders/DebugDraw.h"

class MainDebug {

public:
	void Init(World& world, VinceWindow& Window, PhysicsSystem& physics_system) {
		// Initialize debug features here

		//Spawn Skybox
		auto SkyboxActor = std::make_unique<AStaticMesh>("Assets/Textures/sky.obj", "Sky");
		SkyboxTexture = Texture("Assets/Textures/sky.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
		StaticMeshComponent* SkyboxMeshComponent = dynamic_cast<StaticMeshComponent*>(SkyboxActor->GetComponentByIndex(0));
		SkyboxMeshComponent->ColorTexture = &SkyboxTexture;
		SkyboxActor->SetWorldScale(glm::vec3(5.0f, 5.0f, 5.0f));
		world.AddActor(std::move(SkyboxActor));

	//Spawn Light
		auto lightActor = std::make_unique<APointLight>();
		LightComponent* light = dynamic_cast<LightComponent*>(lightActor->GetComponentByIndex(0));
		world.AddActor(std::move(lightActor));

		//Spawn Dragon, With physics body and textures
		auto DragonMeshActor = std::make_unique<AStaticMesh>("Assets/Models/Dragon/model2.obj", "Default");
		ColorTexture = Texture("Assets/Models/Dragon/Color.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
		NormalTexture = Texture("Assets/Models/Dragon/Normal.png", GL_TEXTURE_2D, GL_TEXTURE1, GL_RGB, GL_UNSIGNED_BYTE);
		DragonMeshActor->ActorTags.push_back("Dragon");
		JPH::BoxShapeSettings boxShapeSettings(JPH::Vec3(1.0f, 0.4f, 1.0f));
		JPH::ShapeSettings::ShapeResult shapeResult = boxShapeSettings.Create();
		JPH::Ref<JPH::Shape> shape = shapeResult.Get();
		JPH::BodyCreationSettings bodySettings(shape, JPH::RVec3(0, 5, 0), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, JPH::ObjectLayer(0));
		auto rigidBodyComponent = std::make_shared<RigidBodyComponent>(&physics_system, bodySettings);
		DragonMeshActor->AddComponent(rigidBodyComponent);
		StaticMeshComponent* DragonmeshComponent = dynamic_cast<StaticMeshComponent*>(DragonMeshActor->GetComponentByIndex(0));
		DragonmeshComponent->ColorTexture = &ColorTexture;
		DragonmeshComponent->NormalTexture = &NormalTexture;
		world.AddActor(std::move(DragonMeshActor));
		
		
		//Spawn Floor, With static physics body
		auto floorMeshActor = std::make_unique<AStaticMesh>("Assets/Models/floor.obj", "Color");
		StaticMeshComponent* floormeshComponent = dynamic_cast<StaticMeshComponent*>(floorMeshActor->GetComponentByIndex(0));
		floormeshComponent->ColorTexture = &ColorTexture;
		floormeshComponent->NormalTexture = &NormalTexture;
		JPH::BoxShapeSettings boxShapeSettings2(JPH::Vec3(100.0f, 0.1f, 100.0f));
		JPH::ShapeSettings::ShapeResult shapeResult2 = boxShapeSettings2.Create();
		JPH::Ref<JPH::Shape> shape2 = shapeResult2.Get();
		JPH::BodyCreationSettings bodySettings2(shape2, JPH::RVec3(0, -5, 0), JPH::Quat::sIdentity(), JPH::EMotionType::Static, JPH::ObjectLayer(1));
		auto rigidBodyComponent2 = std::make_shared<RigidBodyComponent>(&physics_system, bodySettings2);
		floorMeshActor->AddComponent(rigidBodyComponent2);
		world.AddActor(std::move(floorMeshActor));
		

	}
	void Update(Camera& camera, World& world, VinceWindow& window) {
		// Update debug features here (Main loop)
		
		//Draw basic shape of gizmo
		Debug::drawLine({ 0, 0, 0 }, { 0, 0.5, 0 }, { 0.0, 1.0, 0.0, 1.0 }, 2, camera, window);
		Debug::drawLine({ 0, 0, 0 }, { 0.5, 0, 0 }, { 1.0, 0.0, 0.0, 1.0 }, 2, camera, window);
		Debug::drawLine({ 0, 0, 0 }, { 0, 0, 0.5 }, { 0.0, 0.0, 1.0, 1.0 }, 2, camera, window);
		Debug::DrawPoint({ 0, 2, 0 }, { 0.0, 1.0, 0.0, 1.0 }, 40, camera, window);
		Debug::DrawPoint({ 2, 0, 0 }, { 1.0, 0.0, 0.0, 1.0 }, 40, camera, window);
		Debug::DrawPoint({ 0, 0, 2 }, { 0.0, 0.0, 1.0, 1.0 }, 40, camera, window);
		
	}


	Texture ColorTexture;
	Texture NormalTexture;
	Texture SkyboxTexture;

};