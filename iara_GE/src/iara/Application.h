#pragma once
#include "core.h"
#include "Log.h"
#include "window.h"
#include "LayerStack.h"

#include "iara/events/Event.h"
#include "iara/events/AppEvent.h"

namespace iara {

	class IARA_API Application {
	public:
		Application();
		virtual ~Application();

		void Run();

		void onEvent(Event& e);

		inline Window& getWindow() { return *m_Window; }
		inline static Application& Get(	) { return *s_Instance; }

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* overlay);
	private:
		bool onWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;

		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}