#pragma once

#include "imgui/imgui.h"
#include "iara.h"
#include "Level.h"

class GameLayer : public iara::Layer {
public:
	GameLayer();
	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate(iara::Timestep ts) override;
	virtual void onImGuiRender() override;
	virtual void onEvent(iara::Event& event) override;
	
	bool onMBPressed(MouseButtonPressedEvent& event);
private:
	void CreateCamera(uint32_t width, uint32_t height);
private:
	iara::Ref<iara::VertexArray> m_vertexArray;

	iara::Ref<iara::Shader>	m_shader;
	iara::Ref<iara::Texture2D> m_texture;

	iara::Scope<iara::OrthographicCamera> m_camera;

	Level m_level;

	glm::vec4 m_color = { 0.2f, 0.4f, 0.2f, 1.0f };
	glm::vec2 m_pos = { 0.0f, 0.0f };

	enum class GameState {
		Play = 0, GameOver = 1
	};

	GameState m_game_state = GameState::Play;
};