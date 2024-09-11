workspace "iara_GE"
	architecture "x64"
	
	configurations 
	{
		"Debug",
		"Release"
	}

	startproject "sandbox"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "iara_GE/vendor/glfw/include"
IncludeDir["GLAD"] = "iara_GE/vendor/glad/include"
IncludeDir["IMGUI"] = "iara_GE/vendor/imgui/"
IncludeDir["GLM"] = "iara_GE/vendor/glm/"
IncludeDir["STB"] = "iara_GE/vendor/stb_image/"

include "iara_GE/vendor/glfw"
include "iara_GE/vendor/glad"
include "iara_GE/vendor/imgui"

project "iara_GE" 
	location "iara_GE"
	kind "StaticLib"
	cppdialect "C++17"
	language "C++"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ir_pch.h"
	pchsource "iara_GE/src/ir_pch.cpp"

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/Utils/**.h",
		"%{prj.name}/vendor/Utils/**.save"
	}

	includedirs
	{
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.IMGUI}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.STB}",
		"%{prj.name}/vendor/Utils/"
	}

	links {
		"GLFW",
		"GLAD",
		"IMGUI",
		"opengl32.lib"
		
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"IARA_PLATFORM_WINDOWS",
			"IARA_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"_CRT_SECURE_NO_WARNINGS"
		}
		

	filter "configurations:Debug" 
		defines "IARA_DEBUG"
		symbols "on"
		runtime "Debug"

	filter "configurations:Release" 
		defines "IARA_RELEASE"
		optimize "on"
		runtime "Release"


project "sandbox" 
	location "sandbox"
	kind "ConsoleApp"
	cppdialect "C++17"
	language "C++"
	staticruntime "on"

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
		"iara_GE/vendor",
		"iara_GE/src",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.GLFW}"
	}

	links
	{
		"iara_GE"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines 
		{
			"IARA_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug" 
		defines "IARA_DEBUG"
		symbols "on"
		runtime "Debug"

		defines 
		{
			"IARA_PROFILE"
		}

	filter "configurations:Release" 
		defines "IARA_RELEASE"
		runtime "Release"
		optimize "on"
	