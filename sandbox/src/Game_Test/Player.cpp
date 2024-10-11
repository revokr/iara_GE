#include "Player.h"

void Player::loadAssets() {
	m_tex = Texture2D::CreateRef("Assets/Textures/horse.png");
}

void Player::onUpdate(Timestep ts) {
	if (Input::IsKeyPressed(IARA_KEY_SPACE)) {
		m_velocity.y += m_engine_power;
		if (m_velocity.y < 0.0f)
			m_velocity.y += m_engine_power * 1.3f;
	}
	else {
		m_velocity.y -= m_gravity;
	}

	m_velocity.y = glm::clamp(m_velocity.y, -20.0f, 20.0f);
	m_pos += m_velocity * (float)ts;
}

void Player::onRender() {
	//Renderer2D::drawQuad(m_pos, { 1.5f, 2.3f }, color::Red);
	Renderer2D::drawQuadRT({ m_pos.x, m_pos.y, 0.0f }, { 1.0f, 1.3f }, glm::radians(getRotation()), m_tex);
}

void Player::onImGuiRender() {

}

void Player::reset() {
	m_pos = { -10.0f, 0.0f };
	m_velocity = { 8.0f, 0.0f };
}
