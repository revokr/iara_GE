#include "ir_pch.h"

#include "iara/Core/Application.h"
#include "iara/events/AppEvent.h"
#include "iara/events/KeyEvent.h"

#include "Input.h"

namespace iara {

	Application* Application::s_Instance = nullptr;

	

	Application::Application() {
		IARA_PROFILE_FUNCTION();

		IARA_CORE_ASSERT(!s_Instance, "Application already exists!!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
		m_Window->SetVSync(true);

		Renderer::Init();
	
		m_imgui_layer = new ImGuiLayer();
		pushOverlay(m_imgui_layer);
	}


	Application::~Application() {
		
	}

	void Application::onEvent(Event& e) {
		IARA_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(IARA_BIND_EVENT_FN(Application::onWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(IARA_BIND_EVENT_FN(Application::onWindowResize)); 
		
		{

			IARA_PROFILE_SCOPE("Layer Event loop");
			for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
				if (e.m_Handled) break;
				(*it)->onEvent(e);
			}
		}
	}

	void Application::pushLayer(Layer* layer)
	{
		IARA_PROFILE_FUNCTION();

		m_LayerStack.pushLayer(layer);
		layer->onAttach();
	}

	void Application::pushOverlay(Layer* overlay)
	{
		IARA_PROFILE_FUNCTION();

		m_LayerStack.pushOverlay(overlay);
		overlay->onAttach();
	}

	void Application::Run() {
		IARA_PROFILE_FUNCTION();

		while(m_Running) {
			float time = Platform::getTime();
			Timestep timestep(time - m_last_frame_time);
			m_last_frame_time = time;

			if (!m_minimized) {
				IARA_PROFILE_SCOPE("Layer loop");
				for (Layer* layer : m_LayerStack) layer->onUpdate(timestep);

				m_imgui_layer->begin();
				for (Layer* layer : m_LayerStack) layer->onImGuiRender();
				m_imgui_layer->end();
			}
			m_Window->onUpdate();
		}
	}

	bool Application::onWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}
	bool Application::onWindowResize(WindowResizeEvent& e)
	{
		if (e.getWidth() == 0 || e.getHeight() == 0) {
			m_minimized = true;
			return false;
		}

		m_minimized = false;
		Renderer::onWindowResize(e.getWidth(), e.getHeight());

		return false;
	}
}