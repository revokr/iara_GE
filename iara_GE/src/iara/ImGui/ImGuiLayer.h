#pragma once

#include "iara/Core/Layer.h"
#include "iara/events/MouseEvent.h"
#include "iara/events/KeyEvent.h"
#include "iara/events/AppEvent.h"

namespace iara {

	class IARA_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void onAttach() override;
		virtual void onDetach() override;
		virtual void onEvent(Event& e) override;

		void begin();
		void end();

		void BlockEvents(bool block) { m_block_events = block; }
	private:
		void setDarkThemeColors();
	private:
		bool m_block_events = true;	
		float m_Time = 0.0f;
	};
}