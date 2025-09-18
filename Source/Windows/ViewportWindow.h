#pragma once
#include "../World/Camera.h"
#include "../World/World.h"

class ViewportWindow {
public:
	ViewportWindow() = default;
	~ViewportWindow() = default;

	void Draw(float& window_width, float& window_height, Camera& camera, VinceWindow& window) //Render Viewport
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

	bool wireframe = false;
	
};
