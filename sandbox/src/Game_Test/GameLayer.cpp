#include "GameLayer.h"

#include <glm/gtc/matrix_transform.hpp>
#include "platform/openGL/OpenGLShader.h"


GameLayer::GameLayer() : Layer{"Sandbox2D"}
{
	auto& window = Application::Get().getWindow();
	CreateCamera(window.GetWidth(), window.GetHeight());

	Random::Init();
}

void GameLayer::onAttach() {
	m_level.init();
}

void GameLayer::onDetach() {

}

void GameLayer::onUpdate(iara::Timestep ts) {
	// Update
	if (m_level.isGameOver()) m_game_state = GameState::GameOver;

	switch (m_game_state) {
		case GameState::Play:
		{
			m_level.onUpdate(ts);
			break;
		}
	}
	
	// Render
	iara::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.2f, 0.2f, 1.0f));
	iara::RenderCommand::Clear();

	glm::vec2 playerPos = m_level.getPlayer().getPosition();

	m_camera->setPosition({ playerPos.x, playerPos.y, 0.0f });

	iara::Renderer2D::BeginScene(*m_camera);

	m_level.onRender();

	iara::Renderer2D::EndScene();
}

void GameLayer::onImGuiRender() {
	ImGui::Begin("custom col");
	ImGui::ColorEdit4("Color", &m_color.x);
	//ImGui::DragFloat2("Position", &m_pos.x);
	ImGui::SliderFloat2("Position", &m_pos.x, -3.0f, 3.0f);
	ImGui::End();
}

void GameLayer::onEvent(iara::Event& event) {
	EventDispatcher dispathcer(event);
	dispathcer.Dispatch<MouseButtonPressedEvent>(IARA_BIND_EVENT_FN(GameLayer::onMBPressed));
}

bool GameLayer::onMBPressed(MouseButtonPressedEvent& event) {
	if (m_game_state == GameState::GameOver)
		m_level.reset();

	m_game_state = GameState::Play;
	return false;
}

void GameLayer::CreateCamera(uint32_t width, uint32_t height) {
	float aspect_ratio = (float)width / (float)height;

	float camWidth = 8.0f;
	float bottom = -camWidth;
	float top = camWidth;
	float left = bottom * aspect_ratio;
	float right = top * aspect_ratio;
	m_camera = CreateScope<OrthographicCamera>(left, right, bottom, top);
}
