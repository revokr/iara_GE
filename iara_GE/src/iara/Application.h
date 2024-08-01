#pragma once
#include "core.h"
#include "Log.h"

namespace iara {

	class IARA_API Application {
	private:
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}