#pragma once

#include "iara.h"

namespace iara {

	class EditorLayer : public iara::Layer {
	public:
		EditorLayer();
		virtual void onAttach() override;
		virtual void onDetach() override;
		virtual void onUpdate(iara::Timestep ts) override;
		virtual void onImGuiRender() override;
		virtual void onEvent(iara::Event& event) override;
	private:

	private:
		iara::OrthographicCameraController m_camera;

		iara::Ref<iara::Texture2D> m_checkerboard;
		iara::Ref<iara::Texture2D> m_checkerboard2;

		iara::Ref<iara::Framebuffer> m_framebuffer;
		glm::vec2 m_viewportSize;

		glm::vec4 m_color = { 0.2f, 0.4f, 0.2f, 1.0f };
		glm::vec2 m_pos = { 0.0f, 0.0f }, m_scale = { 0.8f, 0.9f };
	};
}