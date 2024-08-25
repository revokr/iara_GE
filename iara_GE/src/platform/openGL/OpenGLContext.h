#pragma once
#include "iara/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace iara {

	class OpenGLContext : public GraphicsContext {
	public:
		OpenGLContext(GLFWwindow* wndHandle); 

		virtual void Init()		   override;
		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_wndHandle;
	};

}