#include <iostream>
#include <stdio.h>
#include <chrono>  
#include "Package/VP/FileDatabase.h"

#include "Package/VP/UI/GUI.h"

#include "Utils/JoltHelpers.h"

#include "World/World.h"
#include "World/Camera.h"

#include "GlobalSettings.h"

#include "Windows/Content/ContentWindow.h"
#include "Windows/DetailsPannel.h"
#include "Windows/WorldSettingsWindow.h"
#include "Windows/ViewportWindow.h"
#include "Windows/Log.hpp"
#include "Windows/Window.h"

#include "Temporary/Debug.h"

#include "Actors/Meshes/aid_model.h"


//#include "Package/VP/UI/Pages/PackageManager/FileBrowser/FileBrowser.h"
//#include "Package/VP/UI/GUI.h"




// Main code
int main(int, char**)
{
	FileDatabase::Load("FileNameDatabase.ini");
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

	World world;

	MainDebug debug;
	debug.Init(world, globals::window1, physics_system);

	//Add aid_model to world
	std::unique_ptr<Aid_Model> model = std::make_unique<Aid_Model>();
	world.AddActor(std::move(model));

	world.ConstructWorld();

	globals::window1.InitFrameBuffer();
	globals::window1.SetupImGuiIO();

	// Initialize Engine Windows
	//ContentWindow Content;
	DetailsWindow Details;
	WorldSettingsWindow worldsettings;
	ViewportWindow viewportWindow;
	
	Log("Starting Main Loop...", EType::Success);

	glEnable(GL_DEPTH_TEST);

	Camera camera(window_width, window_height, glm::vec3(0.0f, 0.0f, 2.0f));
	globals::cam = &camera;

	GUI m_GUI;

	using clock = std::chrono::high_resolution_clock;
	auto lastTime = clock::now();
	while (!glfwWindowShouldClose(globals::window1.getWindow()))
	{
		// Calculate delta time
		auto currentTime = clock::now();
		std::chrono::duration<float> elapsed = currentTime - lastTime;
		float deltaTime = elapsed.count();
		lastTime = currentTime;

		glfwPollEvents();
		if (glfwGetWindowAttrib(globals::window1.getWindow(), GLFW_ICONIFIED) != 0)
		{
			//ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		globals::window1.NewFrame();

		//Render Viewport
		viewportWindow.Draw(window_width, window_height, camera, globals::window1);
		

		//Render Engine Windows
		worldsettings.Draw(world, globals::window1, physics_system);
		DrawLogAdvanced();
		//Content.RenderContentWindow();
		world.RenderWorldOutliner();
		Details.RenderDetailsWindow(world);

		if (!m_GUI.HasInitialized) {
			m_GUI.init();
		}

		m_GUI.render();

		//Render ImGui
		ImGui::Render();

		//Begin rendering to viewport frame-buffer
		globals::window1.getFrameBuffer()->Bind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		camera.Inputs(globals::window1.getWindow());
		camera.updateMatrix(45.0f, 0.01f, 100000.0f);

		// Update the physics
		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable.
		const int cCollisionSteps = 2;
		physics_system.Update(deltaTime, cCollisionSteps, &temp_allocator, &job_system);

		world.TickWorld(deltaTime);
		world.Render(&globals::window1, &camera);

		//debug.Update(camera, world, globals::window1);

		globals::window1.EndFrame();
	}

	FileDatabase::Save("FileNameDatabase.ini");

	// Cleanup
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		globals::window1.getFrameBuffer()->Delete();

		glfwDestroyWindow(globals::window1.getWindow());
		glfwTerminate();
	}

	return 0;
}