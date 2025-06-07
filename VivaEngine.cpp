#include <iostream>
#include <stdio.h>

// Include VivaEngine Shaders
#include "Shaders/ShaderClass.h"
#include "Shaders/VBO.h"
#include "Shaders/VAO.h"
#include "Shaders/EBO.h"
#include "Shaders/Texture.h"

#include "Camera.h"
#include "Model.h"
#include "Mesh.h"

#include "Package/Hot.h"

#include "Windows/Log.hpp"

#include "VinceWindow.h"

#include "World.h"
#include "Actors/Lights/APointLight.h"
#include "Actors/AStaticMesh.h"
#include "Components/MovementComponent.h"
#include <chrono>  

#include "Jolt/Jolt.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Factory.h"
#include "Components/RigidBodyComponent.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"

#include "Jolt/Core/JobSystemThreadPool.h"

#include "JoltHelpers.h"

float window_width = 800;
float window_height = 800;


// Main code
int main(int, char**)
{
	//Register allocation hook.In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
		// This needs to be done before any other Jolt function is called.
	RegisterDefaultAllocator();

	// Install trace and assert callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

		// Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
		// It is not directly used in this example but still required.
		Factory::sInstance = new Factory();

	// Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
	// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
	// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
	RegisterTypes();

	PhysicsSystem physics_system;

	// We need a temp allocator for temporary allocations during the physics update. We're
// pre-allocating 10 MB to avoid having to do allocations during the physics update.
// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
// malloc / free.
	TempAllocatorImpl temp_allocator(10 * 1024 * 1024);

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

	// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint cMaxBodies = 1024;

	// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
	const uint cNumBodyMutexes = 0;

	// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
	// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
	// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
	const uint cMaxBodyPairs = 1024;

	// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
	// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
	// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
	const uint cMaxContactConstraints = 1024;

	// Create mapping table from object layer to broadphase layer
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	// Also have a look at BroadPhaseLayerInterfaceTable or BroadPhaseLayerInterfaceMask for a simpler interface.
	BPLayerInterfaceImpl broad_phase_layer_interface;

	// Create class that filters object vs broadphase layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	// Also have a look at ObjectVsBroadPhaseLayerFilterTable or ObjectVsBroadPhaseLayerFilterMask for a simpler interface.
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

	// Create class that filters object vs object layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	// Also have a look at ObjectLayerPairFilterTable or ObjectLayerPairFilterMask for a simpler interface.
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	// Now we can create the actual physics system.
	physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);


	bool wireframe = false;
	bool unlit = false;

	//Load level from package ""C:\Projects\VinceEngine\vincedata\levels\voodooshop\area_voodooshop\world.hot"
	HotFile map = Hot::ReadFile("C:/Projects/VinceEngine/vincedata/levels/voodooshop/area_voodooshop/world.hot");

	HotFile Verts = Hot::ReadFile(map.Files[0]);

	for(const HotFileInfo& file : Verts.Files) {
		//write file to disk
		std::ofstream outFile("Models/Test/" + file.Name, std::ios::binary);
		if (outFile) {
			outFile.write(reinterpret_cast<const char*>(file.Data.data()), file.Data.size());
			outFile.close();
			Log("Extracted: " + file.Name, EType::Normal);
		} else {
			Log("Failed to extract: " + file.Name, EType::Error);
		}

	}

	//init window here
	VinceWindow window(window_width, window_height, "VinceEngine");

	World world;

	APointLight lightActor;

	world.AddActor(lightActor);

	LightComponent* light = dynamic_cast<LightComponent*>(lightActor.GetComponentByIndex(0));

	AStaticMesh DragonMeshActor("Models/Dragon/model2.obj", "Default");
	Texture ColorTexture("Models/Dragon/Color.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	Texture NormalTexture("Models/Dragon/Normal.png", GL_TEXTURE_2D, GL_TEXTURE1, GL_RGB, GL_UNSIGNED_BYTE);

	// Create a simple box shape for the dragon's collision
	JPH::BoxShapeSettings boxShapeSettings(JPH::Vec3(1.0f, 1.0f, 1.0f));
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
		DragonMeshActor.AddComponent(rigidBodyComponent);

	StaticMeshComponent* DragonmeshComponent = dynamic_cast<StaticMeshComponent*>(DragonMeshActor.GetComponentByIndex(0));
	DragonmeshComponent->ColorTexture = &ColorTexture;
	DragonmeshComponent->NormalTexture = &NormalTexture;

	//auto movementComponent = std::make_shared<MovementComponent>();
	//DragonMeshActor.AddComponent(movementComponent);

	world.AddActor(DragonMeshActor);

	AStaticMesh SkyboxActor("Textures/sky.obj", "Sky");
	Texture SkyboxTexture("Textures/sky.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);

	StaticMeshComponent* SkyboxMeshComponent = dynamic_cast<StaticMeshComponent*>(SkyboxActor.GetComponentByIndex(0));
	SkyboxMeshComponent->ColorTexture = &SkyboxTexture;

	world.AddActor(SkyboxActor);

	SkyboxActor.SetWorldScale(glm::vec3(5.0f, 5.0f, 5.0f));


	AStaticMesh floorMeshActor("Models/floor.obj", "Default");
	StaticMeshComponent* floormeshComponent = dynamic_cast<StaticMeshComponent*>(floorMeshActor.GetComponentByIndex(0));
	floormeshComponent->ColorTexture = &ColorTexture;
	floormeshComponent->NormalTexture = &NormalTexture;

	// Create a simple box shape for the dragon's collision
	JPH::BoxShapeSettings boxShapeSettings2(JPH::Vec3(100.0f, 0.2f, 100.0f));
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
	floorMeshActor.AddComponent(rigidBodyComponent2);


	world.AddActor(floorMeshActor);



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
				DragonMeshActor.SetWorldPosition(glm::vec3(0.0f, 0.0f, 0.0f));
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


		// Update the physics
		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
		const int cCollisionSteps = 1;

		// Step the world
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