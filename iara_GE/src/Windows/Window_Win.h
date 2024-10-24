#pragma once

#include "iara/events/AppEvent.h"
#include "iara/events/KeyEvent.h"
#include "iara/events/MouseEvent.h"
#include "iara/Renderer/GraphicsContext.h"
#include "iara/Core/window.h"
#include <GLFW/glfw3.h>


namespace iara {

	class IARA_API Window_Win : public Window {
	public:
		Window_Win(const WindowProps& props);
		virtual ~Window_Win();

		void onUpdate() override;

		inline uint32_t GetHeight() const override {return m_Data.Height; }
		inline uint32_t GetWidth() const override {return m_Data.Width; }

		inline void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		inline virtual void* GetNativeWindow() const { return m_Window; }

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;

		GraphicsContext* m_context;

		struct WindowData {
			std::string Title;
			unsigned int Height, Width;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};	

}