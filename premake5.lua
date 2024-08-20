workspace "iara_GE"
	architecture "x64"
	
	configurations 
	{
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "iara_GE/vendor/glfw/include"
IncludeDir["GLAD"] = "iara_GE/vendor/glad/include"
--IncludeDir["IMGUI"] = "iara_GE/vendor/imgui/"
IncludeDir["GLM"] = "iara_GE/vendor/glm/"

include "iara_GE/vendor/glfw"
include "iara_GE/vendor/glad"
--include "iara_GE/vendor/imgui"

project "iara_GE" 
	location "iara_GE"
	kind "SharedLib"
	language "C++"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ir_pch.h"
	pchsource "iara_GE/src/ir_pch.cpp"

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
		--"%{prj.name}/vendor/glm/glm/**.hpp",
		--"%{prj.name}/vendor/glm/glm/**.inl"
	}

	includedirs
	{
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		--"%{IncludeDir.IMGUI}",
		"%{IncludeDir.GLM}"
	}

	links {
		"GLFW",
		"GLAD",
		--"IMGUI",
		"opengl32.lib",
		"dwmapi.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines 
		{
			"IARA_PLATFORM_WINDOWS",
			"IARA_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/sandbox")
		}

		

	filter "configurations:Debug" 
		defines "IARA_DEBUG"
		buildoptions "/MDd"
		symbols "On"
		staticruntime "off"
		runtime "Debug"

	filter "configurations:Release" 
		defines "IARA_RELEASE"
		buildoptions "/MD"
		symbols "On"


project "sandbox" 
	location "sandbox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "On"

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
		"iara_GE/src",
		"%{IncludeDir.GLM}"
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
		buildoptions "/MDd"
		symbols "On"
		staticruntime "off"
		runtime "Debug"

	filter "configurations:Release" 
		defines "IARA_RELEASE"
		symbols "On"
	