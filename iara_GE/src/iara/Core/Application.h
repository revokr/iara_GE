#pragma once
#include "core.h"
#include "Log.h"
#include "window.h"
#include "iara/Core/LayerStack.h"
#include "iara/Renderer/Shader.h"

#include "iara/Core/Timestep.h"
#include "iara/events/Event.h"
#include "iara/events/AppEvent.h"
#include "iara/ImGui/ImGuiLayer.h"
#include "iara/Renderer/Buffer.h"
#include "iara/Renderer/VertexArray.h"
#include "iara/Renderer/Renderer.h"

#include "Windows/Platform.h"

struct AppCommandLineArgs {
	int count = 0;
	char** args = nullptr;

	const char* operator[](int index) const {
		return args[index];
	}
};

namespace iara {

	class IARA_API Application {
	public:
		Application(AppCommandLineArgs args = AppCommandLineArgs());
		virtual ~Application();

		virtual void Run();
		void Close();

		virtual void onEvent(Event& e);

		inline Window& getWindow() { return *m_Window; }
		inline static Application& Get(	) { return *s_Instance; }

		inline ImGuiLayer* getImguiLayer() { return m_imgui_layer; }

		virtual void pushLayer(Layer* layer);
		virtual void pushOverlay(Layer* overlay);

		AppCommandLineArgs getCommandLineArgs() const { return m_args; }
	private:
		bool onWindowClose(WindowCloseEvent& e);
		bool onWindowResize(WindowResizeEvent& e);
	private:
		AppCommandLineArgs m_args;
		Scope<Window> m_Window;
		ImGuiLayer* m_imgui_layer;
		bool m_Running = true;
		bool m_minimized = false;
		LayerStack m_LayerStack;
		float m_last_frame_time = 0.0f;

		static Application* s_Instance;
	};

	/// To be defined in CLIENT
	Application* CreateApplication(AppCommandLineArgs args);
}