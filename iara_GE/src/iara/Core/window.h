#pragma once

#include "ir_pch.h"

#include "iara/Core/core.h"
#include "iara/events/Event.h"


namespace iara {

	struct WindowProps {
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps(const std::string& title = "IARA Engine",
			unsigned int width = 1280,
			unsigned int height = 720) :
			Title{ title }, Width{ width }, Height{ height } {}
	};

	class IARA_API Window {
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {};

		virtual void onUpdate() = 0;

		virtual unsigned int GetHeight() const = 0;
		virtual unsigned int GetWidth()  const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}