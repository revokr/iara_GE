#pragma once

#include "../core.h"

namespace iara {

	enum class EventType {
		None = 0, 
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, 
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
		virtual std::string ToString() const { return getName(); }

		inline bool isInCategory(EventCategory category) {
			return getCategoryFlags() & category;
		}

	protected:
		bool m_Handled = false;
	};


	class EventDispatcher {
		template <typename T> 
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event) :
			m_Event{ event } {}

		/*template<typename T>
		bool Dispatch(EventFn<T> func) {
			if (m_Event.getEventType() == T::getStaticType()) {
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}*/

	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& o, Event& e) {
		return o << e.ToString();
	}


}