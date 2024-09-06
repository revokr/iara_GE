#include "Sandbox2D.h"

#include <imgui/imgui.h>

#include <chrono>

namespace color {
	static glm::vec4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	static glm::vec4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	static glm::vec4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
}

Sandbox2D::Sandbox2D()
	: m_camera{1280.0f / 720.0f, true}
{

}

void Sandbox2D::onAttach() {
	IARA_PROFILE_FUNCTION();

	m_checkerboard = iara::Texture2D::Create("Assets/Textures/check4.png");
	
}

void Sandbox2D::onDetach() {

}

void Sandbox2D::onUpdate(iara::Timestep ts) {

	IARA_PROFILE_FUNCTION();
	
	{/// Update
		m_camera.onUpdate(ts);
	}

	/// Render
	{
		IARA_PROFILE_SCOPE("Renderer prep");
		iara::RenderCommand::Clear();
		iara::RenderCommand::SetClearColor({ 0.4f, 0.4f, 0.4f, 1.0f });
	}

	{
		IARA_PROFILE_SCOPE("Renderer draw");
		iara::Renderer2D::BeginScene(m_camera.getCamera());

		iara::Renderer2D::drawQuadRC({ -1.0f,  0.0f }, { 0.8f, 0.8f }, 45.0f, { 0.8f, 0.3f, 0.2f, 1.0f });
		iara::Renderer2D::drawQuadC({ 0.5f, -0.5f }, m_scale, m_color);
		iara::Renderer2D::drawQuadT({ 0.0f,  0.0f, -0.1f }, { 20.0f, 20.0f }, m_checkerboard, 5.0f);	

		iara::Renderer2D::EndScene();
	}
}

void Sandbox2D::onImGuiRender() {
	IARA_PROFILE_FUNCTION();

	ImGui::Begin("Profile Results");

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
		1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::ColorEdit3("color", &m_color.r);
	ImGui::SliderFloat2("Size", &m_scale.x, 0.1f, 10.0f);

	ImGui::End();

}

void Sandbox2D::onEvent(iara::Event& event) {
	m_camera.onEvent(event);
}

