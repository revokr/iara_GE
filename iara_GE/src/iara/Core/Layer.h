#pragma once

#include "iara/Core/core.h"
#include "iara/events/Event.h"
#include "ir_pch.h"
#include "iara/Core/Timestep.h"

namespace iara {

	class IARA_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onUpdate(Timestep ts) {}
		virtual void onImGuiRender() {}
		virtual void onEvent(Event& event) {}

		const std::string& GetName() const { return m_Name; }
	protected:
		std::string m_Name;
	};
}