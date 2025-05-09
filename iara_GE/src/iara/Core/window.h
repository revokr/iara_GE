#pragma once

#include "ir_pch.h"

#include "iara/Core/core.h"
#include "iara/events/Event.h"


namespace iara {

	struct WindowProps {
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "IARA Engine",
			uint32_t width = 1700,
			uint32_t height = 950) :
			Title{ title }, Width{ width }, Height{ height } {}
	};

	class IARA_API Window {
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {};

		virtual void onUpdate() = 0;

		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetWidth()  const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}