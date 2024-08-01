#include "ir_pch.h"

#include "Application.h"
#include "events/AppEvent.h"
#include "events/KeyEvent.h"

namespace iara {

	Application::Application() {

	}

	Application::~Application() {

	}

	void Application::Run() {
		WindowResizeEvent ev(1280, 720);
		IARA_TRACE(ev.ToString());
		KeyPressedEvent key(2, 1);
		IARA_WARN(key.ToString());
		while (1);
	}

}