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
	iara::OrthographicCameraController m_camera;

	iara::Ref<iara::Texture2D> m_checkerboard;

	glm::vec4 m_color = { 0.2f, 0.4f, 0.2f, 1.0f };
	glm::vec2 m_pos = { 0.0f, 0.0f }, m_scale = { 0.8f, 0.9f };
};