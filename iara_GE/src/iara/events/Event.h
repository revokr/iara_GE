#pragma once

#include "iara/Core/core.h"

namespace iara {

	enum class EventType {
		None = 0, 
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory {
		None = 0,
		EventCategoryApp		 = 1,
		EventCategoryInput		 = 2,
		EventCategoryKeyboard	 = 4,
		EventCategoryMouse		 = 8,
		EventCategoryMouseButton = 16
	};

#define EVENT_CLASS_TYPE(type) static EventType getStaticType() {return EventType::##type; }\
								   virtual EventType getEventType() const override {return getStaticType(); }\
								   virtual const char* getName() const override { return #type; }

	class IARA_API Event {
		friend class EventDispatcher;
	
	public: 
		virtual EventType getEventType() const = 0;
		virtual const char* getName() const = 0;
		virtual int getCategoryFlags() const = 0;
		virtual std::string ToString() const { return ""; };

		inline bool isInCategory(EventCategory category) {
			return getCategoryFlags() & category;
		}

		bool m_Handled = false;
	};


	class EventDispatcher {
		template <typename T> 
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event) :
			m_Event{ event } {}

		template<typename T>
		bool Dispatch(EventFn<T> func) {
			if (m_Event.getEventType() == T::getStaticType()) {
				m_Event.m_Handled |= func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& o, const Event& e) {
		return o << e.ToString();
	}


}