#include "ir_pch.h"
#include "ImGuiLayer.h"


#include "platform/openGL/imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"
//#include "imgui.h"

#include "../Application.h"

namespace iara {

	ImGuiLayer::ImGuiLayer() : Layer{ "ImGui_Layer" } {
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::onAttach() {
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		ImGui::StyleColorsDark();

		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::onDetach()
	{
	}

	void ImGuiLayer::onUpdate() {

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2(app.getWindow().GetWidth(), app.getWindow().GetHeight());

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
		m_Time = time;

		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ImGuiLayer::onEvent(Event& event) {
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressedEvent>(IARA_BIND_EVENT_FN(ImGuiLayer::onMouseButtonPressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(IARA_BIND_EVENT_FN(ImGuiLayer::onMouseButtonReleased));
		dispatcher.Dispatch<MouseMovedEvent>(IARA_BIND_EVENT_FN(ImGuiLayer::onMouseMoved));
		dispatcher.Dispatch<MouseScrolledEvent>(IARA_BIND_EVENT_FN(ImGuiLayer::onMouseScrolled));
		dispatcher.Dispatch<KeyPressedEvent>(IARA_BIND_EVENT_FN(ImGuiLayer::onKeyPressed));
		dispatcher.Dispatch<KeyTypedEvent>(IARA_BIND_EVENT_FN(ImGuiLayer::onKeyTyped)); 
		dispatcher.Dispatch<KeyReleaseEvent>(IARA_BIND_EVENT_FN(ImGuiLayer::onKeyReleased));
		dispatcher.Dispatch<WindowResizeEvent>(IARA_BIND_EVENT_FN(ImGuiLayer::onWindowResize));

	}

	bool ImGuiLayer::onMouseButtonPressed(MouseButtonPressedEvent& ev) {
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[ev.getMouseButton()] = true;

		return false;
	}

	bool ImGuiLayer::onMouseButtonReleased(MouseButtonReleasedEvent& ev) {
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[ev.getMouseButton()] = false;

		return false;
	}

	bool ImGuiLayer::onMouseMoved(MouseMovedEvent& ev) {
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(ev.getMouseX(), ev.getMouseY());

		return false;
	}

	bool ImGuiLayer::onMouseScrolled(MouseScrolledEvent& ev) {
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheel += ev.getOffsetY();
		io.MouseWheelH += ev.getOffsetX();

		return false;
	}

	bool ImGuiLayer::onKeyPressed(KeyPressedEvent& ev) {
		ImGuiIO& io = ImGui::GetIO();
		//io.KeysDown[ev.getKeyCode()] = true;

		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL || GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT || GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT || GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER || GLFW_KEY_RIGHT_SUPER];

		return false;
	}

	bool ImGuiLayer::onKeyTyped(KeyTypedEvent& ev) {
		ImGuiIO& io = ImGui::GetIO();
		int keycode = ev.getKeyCode();
		if (keycode > 0 && keycode < 0x10000) {
			io.AddInputCharacter((unsigned short)keycode);	
		}

		return false;
	}

	bool ImGuiLayer::onKeyReleased(KeyReleaseEvent& ev)	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[ev.getKeyCode()] = false;

		return false;
	}


	bool ImGuiLayer::onWindowResize(WindowResizeEvent& ev) {
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(ev.getWidth(), ev.getHeight());

		return false;
	}

}