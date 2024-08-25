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

		virtual void onAttach() override;
		virtual void onDetach() override;
		virtual void onImGuiRender() override;

		void begin();
		void end();
	private:


		float m_Time = 0.0f;
	};
}