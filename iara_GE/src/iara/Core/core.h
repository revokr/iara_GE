#pragma once

#include <memory>
#include <intrin.h>

#ifdef IARA_PLATFORM_WINDOWS
#if IARA_DYNAMIC_LINK
	#ifdef IARA_BUILD_DLL
		#define IARA_API __declspec(dllexport)
	#else
		#define IARA_API __declspec(dllimport)
	#endif
#else
	#define IARA_API
#endif
#endif


#define BIT(x) (1 << x)

#ifdef IARA_ASSERTS
	#define IARA_ASSERT(x, ...) { if (!(x)) { 
		IARA_ERROR("Assertion failed {0}", __VA_ARGS__);
		__debugbreak(); }}
	#define IARA_CORE_ASSERT(x, ...) { if (!(x)) { 
			IARA_CORE_ERROR("Assertion failed {0}", __VA_ARGS__);
			__debugbreak(); }}
#else 
	#define IARA_ASSERT(x, ...)
	#define IARA_CORE_ASSERT(x, ...) 
#endif

#define IARA_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace iara {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}