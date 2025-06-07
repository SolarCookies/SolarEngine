#include "VinceWindow.h"
#include "Windows/Log.hpp"
#include "OpenGL_Stuff.h"

void VinceWindow::init()
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		assert(false && "Failed to initialize GLFW");

	// GL 3.0 + GLSL 330
	glsl_version = "#version 330";
	Log("GLSL Version set to: " + std::string(glsl_version), EType::Normal);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	Log("FrameBuffer Initialized", EType::BLUE);

	// Create window with graphics context using unique_ptr and custom deleter
	window = std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)>(
		glfwCreateWindow(frameBuffer.WIDTH, frameBuffer.HEIGHT, "VivaEngine GLFW OpenGL3", nullptr, nullptr),
		&glfwDestroyWindow
	);
	if (!window) {
		Log("Failed to create GLFW window", EType::Error);
		std::cout << "Failed to create GLFW window" << std::endl;
		assert(false && "Failed to create GLFW window");
	}

	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window.get(), &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(window.get());
	Log("GLFW Hooked, Window Created", EType::BLUE);

	glfwSwapInterval(0); // Enable vsync
	//Log("VSync Enabled", EType::Normal);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "Failed to initialize OpenGL context\n");
		Log("Failed to initialize OpenGL context", EType::Error);
		assert(false && "Failed to initialize OpenGL context");
	}

	glViewport(0, 0, bufferWidth, bufferHeight);
}

void VinceWindow::SetupImGuiIO()
{
	// Setup Dear ImGui context
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Game-pad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		 // Enable Docking

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		//add comic sans
		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\comic.ttf", 18.0f);

		// Setup Platform/Renderer back-ends
		ImGui_ImplGlfw_InitForOpenGL(window.get(), true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		//ImGui Style Colors
		{
			ImVec4* colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.00f, 0.15f, 0.94f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.00f, 0.23f, 0.94f);
			colors[ImGuiCol_Border] = ImVec4(0.57f, 0.00f, 1.00f, 0.50f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.04f, 0.00f, 0.03f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.38f, 0.16f, 0.48f, 0.54f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.83f, 0.26f, 0.98f, 0.40f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.98f, 0.26f, 0.94f, 0.67f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.00f, 0.15f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.21f, 0.04f, 0.26f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.00f, 0.23f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.00f, 0.14f, 0.53f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.29f, 0.02f, 0.43f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.75f, 0.06f, 0.76f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.94f, 0.00f, 1.00f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.69f, 0.26f, 0.98f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.83f, 0.24f, 0.88f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.73f, 0.26f, 0.98f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.85f, 0.26f, 0.98f, 0.40f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.62f, 0.26f, 0.98f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.06f, 0.73f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.47f, 0.26f, 0.98f, 0.31f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.59f, 0.26f, 0.98f, 0.80f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.72f, 0.26f, 0.98f, 1.00f);
			colors[ImGuiCol_Separator] = ImVec4(0.47f, 0.03f, 0.44f, 0.50f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.57f, 0.10f, 0.75f, 0.78f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.73f, 0.10f, 0.75f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.77f, 0.26f, 0.98f, 0.20f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.89f, 0.26f, 0.98f, 0.67f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.92f, 0.26f, 0.98f, 0.95f);
			colors[ImGuiCol_InputTextCursor] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.67f, 0.26f, 0.98f, 0.80f);
			colors[ImGuiCol_Tab] = ImVec4(0.46f, 0.18f, 0.58f, 0.86f);
			colors[ImGuiCol_TabSelected] = ImVec4(0.51f, 0.20f, 0.68f, 1.00f);
			colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.66f, 0.26f, 0.98f, 1.00f);
			colors[ImGuiCol_TabDimmed] = ImVec4(0.14f, 0.07f, 0.15f, 0.97f);
			colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.26f, 0.14f, 0.42f, 1.00f);
			colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.31f, 0.13f, 0.50f, 0.00f);
			colors[ImGuiCol_DockingPreview] = ImVec4(0.89f, 0.26f, 0.98f, 0.70f);
			colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.02f, 0.29f, 1.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.95f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.20f, 0.01f, 0.31f, 1.00f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.19f, 0.02f, 0.30f, 1.00f);
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.14f, 0.02f, 0.28f, 1.00f);
			colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
			colors[ImGuiCol_TextLink] = ImVec4(0.41f, 0.02f, 1.00f, 1.00f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.83f, 0.26f, 0.98f, 0.35f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavCursor] = ImVec4(0.44f, 0.26f, 0.98f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		}

		Log("ImGui Initiated and Style Set", EType::BLUE);
	}
}

void VinceWindow::InitFrameBuffer()
{
	//Gets the frame-buffer ready to start
	frameBuffer.create_framebuffer();
	Log("Frame-buffer Created", EType::BLUE);
}
