#include "ir_pch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <gl/GL.h>

namespace iara {

	OpenGLContext::OpenGLContext(GLFWwindow* wndHandle) : m_wndHandle{ wndHandle } {

	}

	void OpenGLContext::Init() {
		glfwMakeContextCurrent(m_wndHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		IARA_CORE_ASSERT(status, "Failed to load Glad!!");

		const unsigned char* s = glGetString(GL_RENDERER);
		std::string rend = std::string(reinterpret_cast<const char*>(s));
		const unsigned char* ss = glGetString(GL_VENDOR);
		std::string vend = std::string(reinterpret_cast<const char*>(ss));
		const unsigned char* sss = glGetString(GL_VERSION);
		std::string vers = std::string(reinterpret_cast<const char*>(sss));
		IARA_CORE_INFO("OpenGL Info:");
		IARA_CORE_INFO("  Renderer: {0}", rend);
		IARA_CORE_INFO("  Vendor: {0}", vend);
		IARA_CORE_INFO("  Version: {0}", vers);
	}

	void OpenGLContext::SwapBuffers() {
		glfwSwapBuffers(m_wndHandle);

	}

}
