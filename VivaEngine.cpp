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

float window_width = 800;
float window_height = 800;

// Main code
int main(int, char**)
{
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
	
	StaticMeshComponent* DragonmeshComponent = dynamic_cast<StaticMeshComponent*>(DragonMeshActor.GetComponentByIndex(0));
	DragonmeshComponent->ColorTexture = &ColorTexture;
	DragonmeshComponent->NormalTexture = &NormalTexture;

	auto movementComponent = std::make_shared<MovementComponent>();
	DragonMeshActor.AddComponent(movementComponent);

	world.AddActor(DragonMeshActor);

	AStaticMesh SkyboxActor("Textures/sky.obj", "Sky");
	Texture SkyboxTexture("Textures/sky.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);

	StaticMeshComponent* SkyboxMeshComponent = dynamic_cast<StaticMeshComponent*>(SkyboxActor.GetComponentByIndex(0));
	SkyboxMeshComponent->ColorTexture = &SkyboxTexture;

	world.AddActor(SkyboxActor);

	SkyboxActor.SetWorldScale(glm::vec3(5.0f, 5.0f, 5.0f));

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