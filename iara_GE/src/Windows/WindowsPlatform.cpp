#include "ir_pch.h"
#include "WindowsPlatform.h"

#include <GLFW/glfw3.h>

namespace iara {

	Platform* Platform::s_Instance = new WindowsPlatform;

	float WindowsPlatform::getTimeImpl() {
		return (float)glfwGetTime();
	}

}
