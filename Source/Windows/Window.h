#pragma once
#ifndef __gl_h_
#include "glad/glad.h"
#endif
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "stb_image/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>
#include <memory>

#include "../Shaders/FrameBuffer.h"

class VinceWindow {
	public:
		VinceWindow(int width, int height, const char* title)
			: width(width),
			height(height),
			title(title),
			window(nullptr, glfwDestroyWindow) // Properly initialize unique_ptr with deleter
		{
			init();
		}
	void init();
	void SetupImGuiIO();
	void InitFrameBuffer();
	void EnableBlur();

	void NewFrame() {
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		glClearColor(0.0f, 0.0f, 0.0f, 0.00f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui::NewFrame();

		// Create a docking space
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);
	}
	void EndFrame() {
		//Stop rendering to viewport frame-buffer
		frameBuffer.Unbind();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window.get());
	}
	GLFWwindow* getWindow() const {
		return window.get();
	}
	FrameBuffer* getFrameBuffer() const {
		return const_cast<FrameBuffer*>(&frameBuffer);
	}
	FrameBuffer* getFrameBuffer2() const {
		return const_cast<FrameBuffer*>(&frameBuffer2);
	}
	const char* glsl_version;

	int width, height;
private:
		const char* title;
		std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)> window; // Use custom deleter type
		FrameBuffer frameBuffer;
		FrameBuffer frameBuffer2; //Used for shadows
};

