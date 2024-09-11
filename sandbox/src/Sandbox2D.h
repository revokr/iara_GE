#pragma once

#include "iara.h"

class Sandbox2D : public iara::Layer {
public:
	Sandbox2D();
	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate(iara::Timestep ts) override;
	virtual void onImGuiRender() override;
	virtual void onEvent(iara::Event& event) override;
private:

private:
	iara::Ref<iara::VertexArray> m_vertexArraySquare;
	iara::Ref<iara::VertexBuffer> vb;

	iara::OrthographicCameraController m_camera;

	iara::Ref<iara::Shader> m_fb_shader;

	iara::Ref<iara::Texture2D> m_checkerboard;
	iara::Ref<iara::Texture2D> m_checkerboard2;

	iara::Ref<iara::Framebuffer> m_framebuffer;

	glm::vec4 m_color = { 0.2f, 0.4f, 0.2f, 1.0f };
	glm::vec2 m_pos = { 0.0f, 0.0f }, m_scale = { 0.8f, 0.9f };
};