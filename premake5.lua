workspace "iara_GE"
	architecture "x64"
	
	configurations 
	{
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "iara_GE" 
	location "iara_GE"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ir_pch.h"
	pchsource "iara_GE/src/ir_pch.cpp"

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/vendor/spdlog/include"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines 
		{
			"IARA_PLATFORM_WINDOWS",
			"IARA_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/sandbox")
		}

		

	filter "configurations:Debug" 
		defines "IARA_DEBUG"
		symbols "On"

	filter "configurations:Release" 
		defines "IARA_RELEASE"
		symbols "On"


project "sandbox" 
	location "sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"iara_GE/vendor/spdlog/include",
		"iara_GE/src"
	}

	links
	{
		"iara_GE"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines 
		{
			"IARA_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug" 
		defines "IARA_DEBUG"
		symbols "On"

	filter "configurations:Release" 
		defines "IARA_RELEASE"
		symbols "On"
	