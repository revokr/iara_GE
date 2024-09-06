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

namespace iara {

	class IARA_API Application {
	public:
		Application();
		virtual ~Application();

		virtual void Run();

		virtual void onEvent(Event& e);

		inline Window& getWindow() { return *m_Window; }
		inline static Application& Get(	) { return *s_Instance; }

		virtual void pushLayer(Layer* layer);
		virtual void pushOverlay(Layer* overlay);

	private:
		bool onWindowClose(WindowCloseEvent& e);
		bool onWindowResize(WindowResizeEvent& e);
	private:
		Scope<Window> m_Window;
		ImGuiLayer* m_imgui_layer;
		bool m_Running = true;
		bool m_minimized = false;
		LayerStack m_LayerStack;
		float m_last_frame_time = 0.0f;

		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}