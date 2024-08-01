#pragma once

#ifdef IARA_PLATFORM_WINDOWS

extern iara::Application* CreateApplication();

int main(int argc, char** argv) {
	iara::Log::init();
	IARA_CORE_ERROR("Core logger initialized!");
	IARA_INFO("Client logger initialized!");

	auto app = CreateApplication();
	app->Run();
	delete app;
}
	
#endif