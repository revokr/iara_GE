#include "ir_pch.h"
#include "ImGuiLayer.h"


#include "GLFW/glfw3.h"
//#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImGuizmo.h"

#include "iara/Core/Application.h"

namespace iara {

	ImGuiLayer::ImGuiLayer() : Layer{ "ImGui_Layer" } {
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::onAttach() {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		io.FontDefault = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Ubuntu/Ubuntu-Bold.ttf", 14.0f);

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.WindowMinSize.x = 2.0f;
		style.WindowMinSize.y = 2.0f;
		style.FramePadding.x = 0.1f;
		style.FramePadding.y = 0.56f;

		setDarkThemeColors();

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.getWindow().GetNativeWindow());

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::onDetach() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::onEvent(Event& e) {
		if (m_block_events) {
			ImGuiIO& io = ImGui::GetIO();
			e.m_Handled |= e.isInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.m_Handled |= e.isInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::begin() {
		IARA_PROFILE_SCOPE("Imgui Begin");

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::end() {
		IARA_PROFILE_SCOPE("Imgui End");
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.getWindow().GetWidth(), (float)app.getWindow().GetHeight());

		//Rendering 
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::setDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		/// /// /// /// TODO

		/// Headers
		colors[ImGuiCol_Header]             = ImVec4{ 0.23f, 0.22f, 0.205f, 1.0f };
		colors[ImGuiCol_HeaderActive]       = ImVec4{ 0.23f, 0.23f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered]      = ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f };
										    
		/// Buttons						    
		colors[ImGuiCol_Button]             = ImVec4{ 0.18f, 0.22f, 0.29f, 1.0f };
		colors[ImGuiCol_ButtonActive]       = ImVec4{ 0.20f, 0.20f, 0.205f, 1.0f };
		colors[ImGuiCol_ButtonHovered]      = ImVec4{ 0.23f, 0.22f, 0.205f, 1.0f };
										    
		/// Frame BG					    
		colors[ImGuiCol_FrameBg]            = ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };
		colors[ImGuiCol_FrameBgActive]      = ImVec4{ 0.23f, 0.24f, 0.225f, 1.0f };
		colors[ImGuiCol_FrameBgHovered]     = ImVec4{ 0.23f, 0.22f, 0.205f, 1.0f };
										    
		/// Tabs						    
		colors[ImGuiCol_Tab]                = ImVec4{ 0.23f, 0.22f, 0.205f, 1.0f };
		colors[ImGuiCol_TabActive]          = ImVec4{ 0.23f, 0.23f, 0.21f, 1.0f };
		colors[ImGuiCol_TabHovered]         = ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f };
		colors[ImGuiCol_TabUnfocused]       = ImVec4{ 0.23f, 0.24f, 0.225f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.23f, 0.24f, 0.225f, 1.0f };

		/// Titles
		colors[ImGuiCol_TitleBg]            = ImVec4{ 0.23f, 0.24f, 0.225f, 1.0f };;
		colors[ImGuiCol_TitleBgActive]      = ImVec4{ 0.19f, 0.19f, 0.19f, 1.0f }; ;
		colors[ImGuiCol_TitleBgCollapsed]   = ImVec4{ 0.23f, 0.22f, 0.205f, 1.0f };
	}

	

}