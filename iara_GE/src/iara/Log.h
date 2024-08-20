#pragma once

#include "core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace iara {

	class IARA_API Log {
	public:
		static void init();

		inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }
		
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	
	};

}



// Core log macros
#define IARA_CORE_FATAL(...)   ::iara::Log::getCoreLogger()->fatal(__VA_ARGS__)
#define IARA_CORE_ERROR(...)   ::iara::Log::getCoreLogger()->error(__VA_ARGS__)
#define IARA_CORE_WARN(...)    ::iara::Log::getCoreLogger()->warn(__VA_ARGS__)
#define IARA_CORE_INFO(...)    ::iara::Log::getCoreLogger()->info(__VA_ARGS__)
#define IARA_CORE_TRACE(...)   ::iara::Log::getCoreLogger()->trace(__VA_ARGS__)

// Client log macros
#define IARA_FATAL(...)        ::iara::Log::getClientLogger()->fatal(__VA_ARGS__)
#define IARA_ERROR(...)        ::iara::Log::getClientLogger()->error(__VA_ARGS__)
#define IARA_WARN(...)         ::iara::Log::getClientLogger()->warn(__VA_ARGS__)
#define IARA_INFO(...)         ::iara::Log::getClientLogger()->info(__VA_ARGS__)
#define IARA_TRACE(...)        ::iara::Log::getClientLogger()->trace(__VA_ARGS__)