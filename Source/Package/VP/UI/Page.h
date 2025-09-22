#pragma once
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class GUI;

class Page {

	public:
	virtual ~Page() = default;
	virtual void render(GUI &gui) = 0;

};