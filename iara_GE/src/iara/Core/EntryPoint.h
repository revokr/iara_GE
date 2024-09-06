#pragma once

#ifdef IARA_PLATFORM_WINDOWS

extern iara::Application* CreateApplication();

int main(int argc, char** argv) {
	iara::Log::init();

	IARA_PROFILE_BEGGIN_SESSION("Startup", "IARA-Startup.json");
	auto app = CreateApplication();
	IARA_PROFILE_END_SESSION();

	IARA_PROFILE_BEGGIN_SESSION("Runtime", "IARA-Runtime.json");
	app->Run();
	IARA_PROFILE_END_SESSION();

	IARA_PROFILE_BEGGIN_SESSION("Shutdown", "IARA-Shutdown.json");
	delete app;
	IARA_PROFILE_END_SESSION();
}
#endif