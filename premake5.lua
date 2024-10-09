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
IncludeDir["IMGUIZMO"] = "iara_GE/vendor/ImGuizmo"
IncludeDir["GLM"] = "iara_GE/vendor/glm/"
IncludeDir["STB"] = "iara_GE/vendor/stb_image/"
IncludeDir["ENTT"] = "iara_GE/vendor/entt/include"
IncludeDir["YAML"] = "iara_GE/vendor/yaml/include"

include "iara_GE/vendor/glfw"
include "iara_GE/vendor/glad"
include "iara_GE/vendor/imgui"
include "iara_GE/vendor/yaml"

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
		"%{prj.name}/src/Debug/**.h",
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/Utils/**.h",
		"%{prj.name}/vendor/Utils/**.save",

		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp"
	}

	includedirs
	{
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.IMGUI}",
		"%{IncludeDir.IMGUIZMO}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.STB}",
		"%{prj.name}/vendor/Utils/",
		"%{IncludeDir.ENTT}",
		"%{IncludeDir.YAML}"
	}

	links {
		"GLFW",
		"GLAD",
		"IMGUI",
		"YAML",
		"opengl32.lib"
	}

	filter "files:iara_GE/vendor/ImGuizmo/**.cpp"
		flags {"NoPCH"}

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
		"%{IncludeDir.ENTT}"
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
	


project "ziara" 
	location "ziara"
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
		"%{IncludeDir.ENTT}",
		"sandbox/Assets",
		"sandbox/Shaders"
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
	