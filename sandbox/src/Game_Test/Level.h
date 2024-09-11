#pragma once

#include "Player.h"
#include "Random.h"

struct Pillar
{
	glm::vec3 TopPosition = { 0.0f, 6.0f, 0.0f };
	glm::vec2 TopScale = { 6.0f, 10.0f };

	glm::vec3 BottomPosition = { 6.0f, 10.0f, 0.0f };
	glm::vec2 BottomScale = { 6.0f, 10.0f };
};

class Level {
public:
	Level() {}

	void init();
	void onUpdate(Timestep ts);
	void onRender();

	void onImGuiRender();

	bool isGameOver() { return m_game_over; }
	void reset();

	Player& getPlayer() { return m_player; }
private:
	void createPillar(int index, float offset);
	void GameOver();
	bool collisionTest();
private:
	Player m_player;

	bool m_game_over = false;

	int m_pillarIndex = 0;
	float m_PillarTarget = 20.0f;
	std::vector<Pillar> m_pillars;
};