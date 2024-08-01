#pragma once

#ifdef IARA_PLATFORM_WINDOWS
	#ifdef IARA_BUILD_DLL
		#define IARA_API __declspec(dllexport)
	#else
		#define IARA_API __declspec(dllimport)
	#endif
#endif 

#define BIT(x) (1 << x)
