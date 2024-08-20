#pragma once

#include "Event.h"

namespace iara {

	class IARA_API KeyEvent : public Event {
	public :
		inline int getKeyCode() const { return m_KeyCode; }

		virtual int getCategoryFlags() const override 
			{ return EventCategory::EventCategoryKeyboard | EventCategory::EventCategoryInput; }
	protected:
		KeyEvent(int keycode) : m_KeyCode{ keycode } {}

		int m_KeyCode;
	};

	class IARA_API KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(int keycode, int repeatCount) :
			KeyEvent(keycode), m_RepeatCount{ repeatCount } {}

		inline int getRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyPressedEvent : " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};

	class IARA_API KeyReleaseEvent : public KeyEvent {
	public:
		KeyReleaseEvent(int keycode) :
			KeyEvent(keycode) {}
	
		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyReleaseEvent : " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)

	};

	class IARA_API KeyTypedEvent : public KeyEvent {
	public:
		KeyTypedEvent(int keycode) :
			KeyEvent(keycode) {}

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyTypedEvent : " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)

	};
}