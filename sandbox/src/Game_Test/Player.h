//#pragma once
//
//#include "iara.h"
//#include "iara/Renderer/Texture.h"
//
//using namespace iara;
//
//namespace color {
//	static glm::vec4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
//	static glm::vec4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
//	static glm::vec4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
//}
//
//class Player {
//public:
//	Player() {}
//
//	void loadAssets();
//
//	void onUpdate(Timestep ts);
//	void onRender();
//
//	void onImGuiRender();
//	void reset();
//
//	float getRotation() { return m_velocity.y * 4 - 50.0f; }
//	const glm::vec2& getPosition() { return m_pos; }
//private:
//	Ref<Texture2D> m_tex;
//
//	glm::vec2 m_pos = { 0.0f, 0.0f };
//	glm::vec2 m_velocity = { 8.0f, 0.0f };
//
//	float m_gravity = 0.2f, m_engine_power = 0.2f;
//};