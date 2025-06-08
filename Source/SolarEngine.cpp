#include <iostream>
#include <stdio.h>
#include <chrono>  

// Include VivaEngine Shaders
#include "Shaders/ShaderClass.h"
#include "Shaders/VBO.h"
#include "Shaders/VAO.h"
#include "Shaders/EBO.h"
#include "Shaders/Texture.h"

#include "World/World.h"
#include "World/Camera.h"

#include "Windows/Log.hpp"
#include "Windows/Window.h"

#include "Actors/Lights/APointLight.h"
#include "Actors/Meshes/AStaticMesh.h"

#include "Components/Movement/MovementComponent.h"
#include "Components/Movement/RigidBodyComponent.h"

#include "Components/Meshes/GatorMeshComponent.h"

#include "Utils/JoltHelpers.h"

#include "Package/Hot.h"
#include "World/LoadVinceWorld.h"

float window_width = 800;
float window_height = 800;


// Main code
int main(int, char**)
{
	//Jolt Physics Engine Requires this
	RegisterDefaultAllocator();
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)
	Factory::sInstance = new Factory();
	RegisterTypes();
	PhysicsSystem physics_system;
	TempAllocatorImpl temp_allocator(10 * 1024 * 1024);
	JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
	const uint cMaxBodies = 1024;
	const uint cNumBodyMutexes = 0;
	const uint cMaxBodyPairs = 1024;
	const uint cMaxContactConstraints = 1024;
	BPLayerInterfaceImpl broad_phase_layer_interface;
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	// Now we can create the actual Jolt physics system.
	physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);


	bool wireframe = false;
	bool unlit = false;


	std::vector<unsigned char> gatorMeshFile;

	
	//Load level from package ""C:\Projects\VinceEngine\vincedata\levels\voodooshop\area_voodooshop\world.hot"
	HotFile map = Hot::ReadFile("Assets/vincedata/levels/carnival/area_midway/modelsAndAnims.hot");

	HotFile Verts = Hot::ReadFile(map.Files[1]);

	for(const HotFileInfo& file : Verts.Files) {
		if(file.Name.find(".gator") != std::string::npos) {
			gatorMeshFile = file.Data; // Store the gator mesh file data
			Log("Found Gator Mesh: " + file.Name, EType::Normal);
		}

	}

	// --- Before main loop, after loading Verts ---
	std::vector<std::vector<unsigned char>> gatorMeshes;
	std::vector<std::string> gatorMeshNames;
	for (const HotFileInfo& file : Verts.Files) {
		if (file.Name.find(".gator") != std::string::npos) {
			gatorMeshes.push_back(file.Data);
			gatorMeshNames.push_back(file.Name);
			//Log("Found Gator Mesh: " + file.Name, EType::Normal);
		}
	}

	// Use the first mesh by default
	size_t currentMeshIndex = 0;
	// --- Add these variables for timing ---
	float meshSwitchTimer = 0.0f;
	const float meshSwitchInterval = 1.0f;

	//init window here
	VinceWindow window(window_width, window_height, "VinceEngine");

	World world;

	Vince::LoadWorld("Assets/vincedata/levels/frenchquarter/area_mainstreet/world.hot", world);


	auto lightActor = std::make_unique<APointLight>();

	LightComponent* light = dynamic_cast<LightComponent*>(lightActor->GetComponentByIndex(0));

	world.AddActor(std::move(lightActor));

	///*
	auto DragonMeshActor = std::make_unique<AStaticMesh>("Assets/Models/Dragon/model2.obj", "Default");
	Texture ColorTexture("Assets/Models/Dragon/Color.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	Texture NormalTexture("Assets/Models/Dragon/Normal.png", GL_TEXTURE_2D, GL_TEXTURE1, GL_RGB, GL_UNSIGNED_BYTE);

	// Create a simple box shape for the dragon's collision
	JPH::BoxShapeSettings boxShapeSettings(JPH::Vec3(1.0f, 0.4f, 1.0f));
	JPH::ShapeSettings::ShapeResult shapeResult = boxShapeSettings.Create();
		JPH::Ref<JPH::Shape> shape = shapeResult.Get();
		JPH::BodyCreationSettings bodySettings(
			shape,
			JPH::RVec3(0, 5, 0),
			JPH::Quat::sIdentity(),
			JPH::EMotionType::Dynamic,
			JPH::ObjectLayer(0)
		);
		auto rigidBodyComponent = std::make_shared<RigidBodyComponent>(&physics_system, bodySettings);
		DragonMeshActor->AddComponent(rigidBodyComponent);

	StaticMeshComponent* DragonmeshComponent = dynamic_cast<StaticMeshComponent*>(DragonMeshActor->GetComponentByIndex(0));
	DragonmeshComponent->ColorTexture = &ColorTexture;
	DragonmeshComponent->NormalTexture = &NormalTexture;

	//auto movementComponent = std::make_shared<MovementComponent>();
	//DragonMeshActor.AddComponent(movementComponent);


	world.AddActor(std::move(DragonMeshActor));

	auto SkyboxActor = std::make_unique<AStaticMesh>("Assets/Textures/sky.obj", "Sky");
	Texture SkyboxTexture("Assets/Textures/sky.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);

	StaticMeshComponent* SkyboxMeshComponent = dynamic_cast<StaticMeshComponent*>(SkyboxActor->GetComponentByIndex(0));
	SkyboxMeshComponent->ColorTexture = &SkyboxTexture;

	SkyboxActor->SetWorldScale(glm::vec3(5.0f, 5.0f, 5.0f));

	world.AddActor(std::move(SkyboxActor));

	auto floorMeshActor = std::make_unique<AStaticMesh>("Assets/Models/floor.obj", "Color");
	StaticMeshComponent* floormeshComponent = dynamic_cast<StaticMeshComponent*>(floorMeshActor->GetComponentByIndex(0));
	floormeshComponent->ColorTexture = &ColorTexture;
	floormeshComponent->NormalTexture = &NormalTexture;

	// Create a simple box shape for the dragon's collision
	JPH::BoxShapeSettings boxShapeSettings2(JPH::Vec3(100.0f, 0.1f, 100.0f));
	JPH::ShapeSettings::ShapeResult shapeResult2 = boxShapeSettings2.Create();
	JPH::Ref<JPH::Shape> shape2 = shapeResult2.Get();
	JPH::BodyCreationSettings bodySettings2(
		shape2,
		JPH::RVec3(0, -5, 0),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Static,
		JPH::ObjectLayer(1)
	);
	auto rigidBodyComponent2 = std::make_shared<RigidBodyComponent>(&physics_system, bodySettings2);
	floorMeshActor->AddComponent(rigidBodyComponent2);


	world.AddActor(std::move(floorMeshActor));

	auto TestDynamicMeshActor = std::make_unique<Actor>();

	//Add Gator Mesh component to the actor
	auto gatorMeshComponent = std::make_shared<GatorMeshComponent>(gatorMeshFile);
	TestDynamicMeshActor->AddComponent(gatorMeshComponent);

	world.AddActor(std::move(TestDynamicMeshActor));

	world.ConstructWorld();

	window.InitFrameBuffer();
	window.SetupImGuiIO();
	

	Log("Starting Main Loop...", EType::Success);

	glEnable(GL_DEPTH_TEST);

	Camera camera(window_width, window_height, glm::vec3(0.0f, 0.0f, 2.0f));

	//init delta time
	using clock = std::chrono::high_resolution_clock;
	auto lastTime = clock::now();

	while (!glfwWindowShouldClose(window.getWindow()))
	{
		// Calculate delta time
		auto currentTime = clock::now();
		std::chrono::duration<float> elapsed = currentTime - lastTime;
		float deltaTime = elapsed.count();
		lastTime = currentTime;

		glfwPollEvents();
		if (glfwGetWindowAttrib(window.getWindow(), GLFW_ICONIFIED) != 0)
		{
			//ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		window.NewFrame();

		//Render Viewport
		{
			ImGui::Begin("Viewport");

			window_width = ImGui::GetContentRegionAvail().x;
			window_height = ImGui::GetContentRegionAvail().y;
			camera.height = window_height;
			camera.width = window_width;

			if (!wireframe) {
				if (ImGui::Button("Wireframe")) {
					glPolygonMode(GL_FRONT, GL_LINE);
					glPolygonMode(GL_BACK, GL_LINE);
					wireframe = true;
				}
			}
			else {
				if (ImGui::Button("Fillframe")) {
					glPolygonMode(GL_FRONT, GL_FILL);
					glPolygonMode(GL_BACK, GL_FILL);
					wireframe = false;
				}
			}
			if (!unlit) {
				if (ImGui::Button("Unlit")) {
					unlit = true;
				}
			}
			else {
				if (ImGui::Button("Lit")) {
					unlit = false;
				}
			}
			

			window.getFrameBuffer()->rescale_framebuffer(window_width, window_height);
			glViewport(0, 0, window_width, window_height);

			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImGui::GetWindowDrawList()->AddImage(
				(ImTextureID)(intptr_t)window.getFrameBuffer()->texture_id,
				ImVec2(pos.x, pos.y),
				ImVec2(pos.x + window_width, pos.y + window_height),
				ImVec2(0, 1),
				ImVec2(1, 0)
			);

			ImGui::End();
		}

		//Stats Window
		{
			ImGui::Begin("Stats");
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

			vec4 lightColor = light->GetColor();
			if (ImGui::ColorEdit4("Light Color", (float*)&lightColor)) {
				light->SetColor(lightColor);
			}
			vec3 lightPos = light->GetPosition();
			if (ImGui::DragFloat3("Light Position", (float*)&lightPos, 0.1f, -10.0f, 10.0f)) {
				light->SetPosition(lightPos);
			}

			if (ImGui::Button("Respawn")) {
				//Set the dragon's position to 0,0,0
				DragonMeshActor->SetWorldPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				rigidBodyComponent.get()->SetBodyPosition(glm::vec3(0, 0, 0));
			}
			//gravity slider
			float gravity = physics_system.GetGravity().GetY();
			if( ImGui::DragFloat("Gravity", &gravity, 0.1f, -20.0f, 20.0f)) {
				//Set the gravity of the physics system
				physics_system.SetGravity(JPH::Vec3(0, gravity, 0));
			}
			//for each actor in the world, display its name and transform
			ImGui::Text("Actors in World: %d", world.GetActorCount());

			ImGui::End();
		}

		//Log Window
		ImGui::Begin("Log");
		if (ImGui::Button("Clear Log")) {
			ClearLog();
		}
		//Add background color to the log
		ImGui::BeginChild("Log", ImVec2(0, 0), true);
		DrawLog();
		ImGui::EndChild();
		ImGui::End();

		//Render ImGui
		ImGui::Render();

		//Begin rendering to viewport frame-buffer
		window.getFrameBuffer()->Bind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		camera.Inputs(window.getWindow());
		camera.updateMatrix(45.0f, 0.01f, 100000.0f);

		// --- Inside main loop, after deltaTime calculation ---
		meshSwitchTimer += deltaTime;
		if (meshSwitchTimer >= meshSwitchInterval && !gatorMeshes.empty()) {
			meshSwitchTimer = 0.0f;
			currentMeshIndex = (currentMeshIndex + 1) % gatorMeshes.size();
			gatorMeshComponent->LoadGatorMesh(gatorMeshes[currentMeshIndex]);
			//gatorMeshComponent->ReInitializeModel();
			//Log("Switched to Gator Mesh: " + gatorMeshNames[currentMeshIndex], EType::Normal);
		}

		// Update the physics
		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable.
		const int cCollisionSteps = 2;
		physics_system.Update(deltaTime, cCollisionSteps, &temp_allocator, &job_system);

		world.TickWorld(deltaTime);
		world.Render(&window, &camera);

		window.EndFrame();
	}

	// Cleanup
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		window.getFrameBuffer()->Delete();
		//world.Destory();
		ColorTexture.Delete();
		NormalTexture.Delete();
		SkyboxTexture.Delete();

		glfwDestroyWindow(window.getWindow());
		glfwTerminate();
	}

	return 0;
}