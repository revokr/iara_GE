#include "ir_pch.h"

#include "Log.h"

namespace iara {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::logger> Log::s_EditorLogger;

	void Log::init() {
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_CoreLogger = spdlog::stderr_color_mt("IARA");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stderr_color_mt("SANDBOX");
		s_ClientLogger->set_level(spdlog::level::trace);

		s_EditorLogger = spdlog::stderr_color_mt("EDITOR");
		s_EditorLogger->set_level(spdlog::level::trace);
	}

}
