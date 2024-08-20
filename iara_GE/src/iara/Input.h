#pragma once

#include "core.h"

namespace iara {

	class IARA_API Input {
	public:
		inline static bool IsKeyPressed(int keycode) { return s_Inst->IsKeyPressedImpl(keycode); }
		inline static bool IsMouseButtonPressed(int button) { return s_Inst->IsMouseButtonPressedImpl(button); }
		inline static std::pair<float, float> GetMousePosition() { return s_Inst->GetMousePositionImpl(); }
		inline static float GetMouseX() { return s_Inst->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Inst->GetMouseYImpl(); }

	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float,float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

	private:
		static Input* s_Inst;
	};
}