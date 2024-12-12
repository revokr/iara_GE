#include "ir_pch.h"
#include "Window_Win.h"

#include "platform/openGL/OpenGLContext.h"

namespace iara {

	static bool s_GLFWInit = false;

	static void GLFWErrorCallback(int error, const char* desc) {
		IARA_CORE_ERROR("GLFW error ({0}): {1}", error, desc);
	}

	Scope<Window> Window::Create(const WindowProps& props) {
		return CreateScope<Window_Win>(props);
	}

	Window_Win::Window_Win(const WindowProps& props) {
		Init(props);
	}

	Window_Win::~Window_Win() {
		Shutdown();
	}

	void Window_Win::onUpdate() {
		IARA_PROFILE_FUNCTION();

		glfwPollEvents();
		m_context->SwapBuffers();
	}

	void Window_Win::SetVSync(bool enabled) {
		if (enabled) glfwSwapInterval(1);
		else		 glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool Window_Win::IsVSync() const
	{
		return m_Data.VSync;
	}

	void Window_Win::Init(const WindowProps& props) {
		IARA_PROFILE_FUNCTION();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;


		IARA_CORE_INFO("Creating window {0} ({1}, {2})", m_Data.Title, m_Data.Width, m_Data.Height);

		if (!s_GLFWInit) {
			int success = glfwInit();
			IARA_CORE_ASSERT(success, "Failed to initialize GLFW");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInit = true;
		}

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, props.Title.c_str(), nullptr, nullptr);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);

		m_context = new OpenGLContext(m_Window);
		m_context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);
		//glfwWindowHint(GLFW_SAMPLES, 4);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* wnd, int width, int height) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(wnd);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* wnd) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(wnd);

			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* wnd, int key, int scancode, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(wnd);
			
			switch (action) {
				case GLFW_PRESS: 
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleaseEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* wnd, unsigned int c) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(wnd);
			KeyTypedEvent event(c);
			data.EventCallback(event);
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* wnd, int button, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(wnd);

			switch (action) {
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data.EventCallback(event);
				break;
			}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* wnd, double xOff, double yOff) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(wnd);

			MouseScrolledEvent event((float)xOff, (float)yOff);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* wnd, double xPos, double yPos) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(wnd);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
			});
	}

	void Window_Win::Shutdown(){
		IARA_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
	}

}
