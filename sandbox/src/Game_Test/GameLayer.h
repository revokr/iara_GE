#pragma once

#include "imgui/imgui.h"
#include "iara.h"
#include "Level.h"

class GameLayer : public Layer {
public:
	GameLayer();
	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onUpdate(Timestep ts) override;
	virtual void onImGuiRender() override;
	virtual void onEvent(Event& event) override;
	
	bool onMBPressed(MouseButtonPressedEvent& event);
private:
	void CreateCamera(uint32_t width, uint32_t height);
private:
	Ref<VertexArray> m_vertexArray;

	Ref<Shader>	m_shader;
	Ref<Texture2D> m_texture;

	Scope<OrthographicCamera> m_camera;

	Level m_level;

	glm::vec4 m_color = { 0.2f, 0.4f, 0.2f, 1.0f };
	glm::vec2 m_pos = { 0.0f, 0.0f };

	enum class GameState {
		Play = 0, GameOver = 1
	};

	GameState m_game_state = GameState::Play;
};