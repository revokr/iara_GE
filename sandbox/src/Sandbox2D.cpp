#include "Sandbox2D.h"

#include <imgui/imgui.h>
#include <glfw/glfw3.h>
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
    iara::FramebufferSpecification fb_spec;
    fb_spec.width = 1280;
    fb_spec.height = 720;
    m_framebuffer = iara::Framebuffer::Create(fb_spec);

	m_checkerboard = iara::Texture2D::Create("Assets/Textures/check4.png");
	m_checkerboard2 = iara::Texture2D::Create("Assets/Textures/Check2.png");

    m_fb_shader = iara::Shader::Create("fb", "Shaders/framebuffer.vert", "Shaders/framebuffer.frag");
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

    
    m_framebuffer->bind();  
    iara::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.5f, 1.0f });
	iara::RenderCommand::Clear();
	
	//IARA_PROFILE_SCOPE("Renderer draw");
	iara::Renderer2D::BeginScene(m_camera.getCamera());
    
	iara::Renderer2D::drawQuadRT({ 6.0f, 0.0f, 0.16f }, { 3.0f, 3.0f }, rotation, m_checkerboard2, 5.0f);
	iara::Renderer2D::drawQuadT({ 0.0f, 0.0f, 0.1f }, { 25.0f, 25.0f }, m_checkerboard, 3.4f);	
	//iara::Renderer2D::drawQuadTC({ 20.0f, 0.0f, 0.15f }, { 20.0f, 20.0f }, m_checkerboard, color::Red, 3.0f);
	for (float y = -10.0f; y <= 10.0f; y += 1.0f) {
		for (float x = -10.0f; x <= 10.0f; x += 1.0f) {
			glm::vec4 color = { (x + 10.0f) / 20.0f, (y + 10.0f) / 20.0f, 0.4f, 0.75f };
			iara::Renderer2D::drawQuadC({ x, y, 0.15f }, { 0.85f, 0.85f }, color);
		}
	}
    iara::Renderer2D::drawQuadC({ 0.0f, 0.0f, 0.16f }, { 5.85f, 5.85f }, {0.5f, 0.6f, 0.2f, 1.0f});
	iara::Renderer2D::EndScene();
    m_framebuffer->unbind();
   
}

void Sandbox2D::onImGuiRender() {
	IARA_PROFILE_FUNCTION();

    bool dockspace = true;

    if (dockspace) {
        static bool* p_open = new bool(true);
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", p_open, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                if (ImGui::MenuItem("Exit")) iara::Application::Get().Close();
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }



        ImGui::Begin("Profile Results");

        auto stats = iara::Renderer2D::getStats();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Renderer Stats: ");
        ImGui::Text("Draw Calls: %d", stats.draw_calls);
        ImGui::Text("Quads: %d", stats.quad_count);
        ImGui::Text("Total Vertices: %d", stats.GetVertices());
        ImGui::Text("Total Indices: %d", stats.GetIndices());

        uint32_t texID = m_framebuffer->getColorAtt();  // Get the texture from the framebuffer
        ImGui::Image((void*)(intptr_t)texID, ImVec2(1280.f, 720.f));
        
        ImGui::End();


        ImGui::End();
    }
}

void Sandbox2D::onEvent(iara::Event& event) {
	m_camera.onEvent(event);
}

