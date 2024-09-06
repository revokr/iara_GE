#pragma once

#include "Event.h"
#include "iara/Core/core.h"

namespace iara {

	class IARA_API MouseMovedEvent : public Event {
	public:
		MouseMovedEvent(float mousex, float mousey) :
			m_mouseX{ mousex }, m_mouseY{ mousey } {}

		inline float getMouseX() {return m_mouseX; }
		inline float getMouseY() {return m_mouseY; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_mouseX << ", " << m_mouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		virtual int getCategoryFlags() const override{
			return EventCategory::EventCategoryMouse | EventCategory::EventCategoryInput;
		}
	private:
		float m_mouseX;
		float m_mouseY;
	};

	class IARA_API MouseScrolledEvent : public Event {
	public:
		MouseScrolledEvent(float mousex, float mousey) :
			m_offsetX{ mousex }, m_offsetY{ mousey } {}

		inline float getOffsetX() { return m_offsetX; }
		inline float getOffsetY() { return m_offsetY; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_offsetX << ", " << m_offsetY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		virtual int getCategoryFlags() const override {
			return EventCategory::EventCategoryMouse | EventCategory::EventCategoryInput;
		}
	private:
		float m_offsetX;
		float m_offsetY;
	};

	class IARA_API MouseButtonEvent : public Event {
	public:
		inline int getMouseButton() const { return m_Button; }

		virtual int getCategoryFlags() const override {
			return EventCategory::EventCategoryMouse | EventCategory::EventCategoryInput;
		}
	protected:
		MouseButtonEvent(int button) :
			m_Button{button} {}
		
		int m_Button;
	};

	class IARA_API MouseButtonPressedEvent : public MouseButtonEvent {
	public:
		MouseButtonPressedEvent(int button) :
			MouseButtonEvent(button) {}

		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class IARA_API MouseButtonReleasedEvent : public MouseButtonEvent {
	public:
		MouseButtonReleasedEvent(int button) :
			MouseButtonEvent(button) {}

		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};


}