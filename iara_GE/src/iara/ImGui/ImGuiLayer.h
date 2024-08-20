#pragma once

#include "iara/Layer.h"
#include "iara/events/MouseEvent.h"
#include "iara/events/KeyEvent.h"
#include "iara/events/AppEvent.h"

namespace iara {

	class IARA_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void onAttach();
		void onDetach();
		void onUpdate();
		void onEvent(Event& event);
	private:
		bool onMouseButtonPressed(MouseButtonPressedEvent& ev);
		bool onMouseButtonReleased(MouseButtonReleasedEvent& ev);
		bool onMouseMoved(MouseMovedEvent& ev);
		bool onMouseScrolled(MouseScrolledEvent& ev);
		bool onKeyPressed(KeyPressedEvent& ev);
		bool onKeyTyped(KeyTypedEvent& ev);
		bool onKeyReleased(KeyReleaseEvent& ev);
		bool onWindowResize(WindowResizeEvent& ev);
	private:


		float m_Time = 0.0f;
	};
}