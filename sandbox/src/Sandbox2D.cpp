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

	m_checkerboard = iara::Texture2D::CreateRef("Assets/Textures/check4.png");
	m_checkerboard2 = iara::Texture2D::CreateRef("Assets/Textures/Check2.png");
}

void Sandbox2D::onDetach() {

}

void Sandbox2D::onUpdate(iara::Timestep ts) {

	IARA_PROFILE_FUNCTION();
    
	
	{/// Update
		m_camera.onUpdate(ts);
	}

	/// Render
    
    iara::Renderer2D::ResetStats();
	
		//IARA_PROFILE_SCOPE("Renderer prep");
    static float rotation = 0.0f;
    rotation += ts * 30.0f;

    
    iara::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.5f, 1.0f });
    iara::RenderCommand::Clear();
	
	//IARA_PROFILE_SCOPE("Renderer draw");
    iara::Renderer2D::BeginScene(m_camera.getCamera());
    
    iara::Renderer2D::drawQuadRT({ 6.0f, 0.0f, 0.16f }, { 3.0f, 3.0f }, rotation, m_checkerboard2, 5.0f);
    iara::Renderer2D::drawQuadT({ 0.0f, 0.0f, 0.1f }, { 25.0f, 25.0f }, m_checkerboard, 3.4f);
	//Renderer2D::drawQuadTC({ 20.0f, 0.0f, 0.15f }, { 20.0f, 20.0f }, m_checkerboard, color::Red, 3.0f);
	for (float y = -10.0f; y <= 10.0f; y += 1.0f) {
		for (float x = -10.0f; x <= 10.0f; x += 1.0f) {
			glm::vec4 color = { (x + 10.0f) / 20.0f, (y + 10.0f) / 20.0f, 0.4f, 0.75f };
            iara::Renderer2D::drawQuadC({ x, y, 0.15f }, { 0.85f, 0.85f }, color);
		}
	}
    iara::Renderer2D::drawQuadC({ 0.0f, 0.0f, 0.16f }, { 5.85f, 5.85f }, {0.5f, 0.6f, 0.2f, 1.0f});
    iara::Renderer2D::EndScene();
   
}

void Sandbox2D::onImGuiRender() {
	IARA_PROFILE_FUNCTION();

    
    ImGui::Begin("Profile Results");

    auto stats = iara::Renderer2D::getStats();

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Renderer Stats: ");
    ImGui::Text("Draw Calls: %d", stats.draw_calls);
    ImGui::Text("Quads: %d", stats.quad_count);
    ImGui::Text("Total Vertices: %d", stats.GetVertices());
    ImGui::Text("Total Indices: %d", stats.GetIndices());
        
    ImGui::End();
}

void Sandbox2D::onEvent(iara::Event& event) {
	m_camera.onEvent(event);
}

