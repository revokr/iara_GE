#include "Level.h"
#include <glm/gtc/matrix_transform.hpp>

void Level::init() {
	m_player.loadAssets();

	m_pillars.resize(5);
	for (int i = 0; i < 5; i++) {
		createPillar(i, i * 10.0f);
	}
}

void Level::onUpdate(Timestep ts) {
	m_player.onUpdate(ts);

	if (collisionTest()) {
		GameOver();
	}

	if (m_player.getPosition().x > m_PillarTarget) {
		createPillar(m_pillarIndex, m_PillarTarget + 20.0f);
		m_pillarIndex = ++m_pillarIndex % m_pillars.size();
		m_PillarTarget += 10.0f;
	}
}

void Level::onRender() {
	const auto& playerPos = m_player.getPosition();

	Renderer2D::drawQuadC({ playerPos.x,  12.0f }, { 30.0f, 10.0f }, color::Red);
	Renderer2D::drawQuadC({ playerPos.x, -12.0f }, { 30.0f, 10.0f }, color::Red);

	for (auto& pillar : m_pillars) {
		Renderer2D::drawQuadC(pillar.TopPosition, pillar.TopScale, color::Red);
		Renderer2D::drawQuadC(pillar.BottomPosition, pillar.BottomScale, color::Red);
	}

	m_player.onRender();
}

void Level::onImGuiRender() {

}

void Level::reset() {
	m_game_over = false;

	m_player.reset();

	m_PillarTarget = 20.0f;
	m_pillarIndex = 0;
	for (int i = 0; i < 5; i++) {
		createPillar(i, i * 10.0f);
	}
}

void Level::createPillar(int index, float offset) {
	Pillar& pillar = m_pillars[index];
	pillar.TopPosition.x = offset;
	pillar.BottomPosition.x = offset;
	/*pillar.TopPosition.z = index * 0.1f - 0.5f;
	pillar.BottomPosition.z = index * 0.1f - 0.5f + 0.05f;*/

	float center = Random::Float() * 40.0f - 15.5f;

	pillar.TopPosition.y = 10.0f - ((10.0f - center) * 0.2f) + Random::Float() * 2.0f;
	pillar.BottomPosition.y = -10.0f - ((-10.0f - center) * 0.2f) - Random::Float() * 1.5f;
}

void Level::GameOver() {
	m_game_over = true;
}

bool Level::collisionTest() {
	const auto& playerPos = m_player.getPosition();

	for (auto& pillar : m_pillars) {
		if (playerPos.x > pillar.TopPosition.x - pillar.TopScale.x * 0.5f && 
			playerPos.x < pillar.TopPosition.x + pillar.TopScale.x * 0.5f) {
			if (playerPos.y > pillar.TopPosition.y - pillar.TopScale.y * 0.5f ||
				playerPos.y < pillar.BottomPosition.y + pillar.BottomScale.y * 0.5f) {
				return true;
			}
		}
	}

	if (playerPos.y > 12.0f || playerPos.y < -12.0f) {
		return true;
	}

	return false;
}
