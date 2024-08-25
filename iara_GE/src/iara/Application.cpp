#include "ir_pch.h"

#include "Application.h"
#include "events/AppEvent.h"
#include "events/KeyEvent.h"

#include "Input.h"

namespace iara {

	Application* Application::s_Instance = nullptr;

	

	Application::Application() {
		IARA_CORE_ASSERT(!s_Instance, "Application already exists!!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
	
		m_imgui_layer = new ImGuiLayer();
		pushOverlay(m_imgui_layer);
	}


	Application::~Application() {
		
	}

	void Application::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));
		
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
			if (e.m_Handled) break;
			(*it)->onEvent(e);  
		}
	}

	void Application::pushLayer(Layer* layer)
	{
		m_LayerStack.pushLayer(layer);
		layer->onAttach();
	}

	void Application::pushOverlay(Layer* overlay)
	{
		m_LayerStack.pushOverlay(overlay);
		overlay->onAttach();
	}

	void Application::Run() {
		while(m_Running) {
			float time = Platform::getTime();
			Timestep timestep(time - m_last_frame_time);
			m_last_frame_time = time;

			for (Layer* layer : m_LayerStack) layer->onUpdate(timestep);

			m_imgui_layer->begin();
			for (Layer* layer : m_LayerStack) layer->onImGuiRender();
			m_imgui_layer->end();

			auto [x, y] = Input::GetMousePosition();
			//IARA_CORE_TRACE("{0}, {1}", x, y);

			m_Window->onUpdate();
		}
	}

	bool Application::onWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}
}