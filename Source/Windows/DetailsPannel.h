#pragma once
#include <string>
#include <vector>
#include <imgui.h>
#include "../Shaders/Texture.h"
#include "../World/World.h"

class DetailsWindow {
public:
	DetailsWindow() = default;
	~DetailsWindow() = default;
	void RenderDetailsWindow(World& world);
};
