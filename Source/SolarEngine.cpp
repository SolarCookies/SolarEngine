#include <iostream>
#include <stdio.h>
#include <chrono>  
#include <gli/gli.hpp>

#include "Utils/JoltHelpers.h"

#include "World/World.h"
#include "World/Camera.h"

#include "Windows/Content/ContentWindow.h"
#include "Windows/DetailsPannel.h"
#include "Windows/WorldSettingsWindow.h"
#include "Windows/ViewportWindow.h"
#include "Windows/Log.hpp"
#include "Windows/Window.h"

#include "Temporary/Debug.h"
#include "Package/Hot.h"
#include "World/LoadVinceWorld.h"

#include "Package/VP/PKG.h"

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


	//init window here
	VinceWindow window(window_width, window_height, "SolarEngine");

	World world;

	PKG pkg;
	pkg.Load("F://CrackedGames//Viva Pinata//bundles_packages//1.pkg");

	MainDebug debug;
	debug.Init(world, window, physics_system);

	// Debug Vince World "Assets\vincedata\levels\frenchquarter\area_mainstreet\world.hot"
	VinceWorldLoader vince_loader;
	vince_loader.LoadWorld("Assets/vincedata/levels/frenchquarter/area_mainstreet/world.hot", world);

	world.ConstructWorld();

	window.InitFrameBuffer();
	window.SetupImGuiIO();

	// Initialize Engine Windows
	ContentWindow Content;
	DetailsWindow Details;
	WorldSettingsWindow worldsettings;
	ViewportWindow viewportWindow;
	
	Log("Starting Main Loop...", EType::Success);

	glEnable(GL_DEPTH_TEST);

	Camera camera(window_width, window_height, glm::vec3(0.0f, 0.0f, 2.0f));

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
		viewportWindow.Draw(window_width, window_height, camera, window);
		

		//Render Engine Windows
		worldsettings.Draw(world, window, physics_system);
		DrawLogAdvanced();
		Content.RenderContentWindow();
		world.RenderWorldOutliner();
		Details.RenderDetailsWindow(world);

		//Render ImGui
		ImGui::Render();

		//Begin rendering to viewport frame-buffer
		window.getFrameBuffer()->Bind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		camera.Inputs(window.getWindow());
		camera.updateMatrix(45.0f, 0.01f, 100000.0f);

		// Update the physics
		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable.
		const int cCollisionSteps = 2;
		physics_system.Update(deltaTime, cCollisionSteps, &temp_allocator, &job_system);

		world.TickWorld(deltaTime);
		world.Render(&window, &camera);

		debug.Update(camera, world, window);
		
		window.EndFrame();
	}

	// Cleanup
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		window.getFrameBuffer()->Delete();

		glfwDestroyWindow(window.getWindow());
		glfwTerminate();
	}

	return 0;
}