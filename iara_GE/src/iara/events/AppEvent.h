#pragma once

#include "Event.h"
#include "../core.h"

namespace iara {

	class IARA_API WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(unsigned int width, unsigned int height) :
			m_Width{ width }, m_Height{ height } {}

		inline unsigned int getWidth() const { return m_Width; }
		inline unsigned int getHeight() const { return m_Height; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		virtual int getCategoryFlags() const override{
			return EventCategory::EventCategoryApp;
		}
	private:
		unsigned int m_Width, m_Height;
	};

	class IARA_API WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
		virtual int getCategoryFlags() const override {
			return EventCategory::EventCategoryApp;
		}
	};

	class IARA_API AppTickEvent : public Event {
	public:
		AppTickEvent() {}

		EVENT_CLASS_TYPE(AppTick)
		virtual int getCategoryFlags() const override {
			return EventCategory::EventCategoryApp;
		}
	};

	class IARA_API AppUpdateEvent : public Event {
		AppUpdateEvent() {}

		EVENT_CLASS_TYPE(AppUpdate)
			virtual int getCategoryFlags() const override {
			return EventCategory::EventCategoryApp;
		}
	};

	class IARA_API AppRenderEvent : public Event {
	public:
		AppRenderEvent() {}

		EVENT_CLASS_TYPE(AppRender)
			virtual int getCategoryFlags() const override {
			return EventCategory::EventCategoryApp;
		}
	};

}