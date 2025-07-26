#include "EditorLayer.h"

#include <imgui/imgui.h>
#include <ImGuizmo\ImGuizmo.h>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "iara\Scene\SceneSerializer.h" 
#include "iara\Utils\FileDialogsUtils.h"
#include "iara\Math\Math.h"
#include "iara\Core\Timer.h"

#include "iara\Scene\SceneRenderer.h"

static float g_on_update_time = 0.0f;
static float g_on_imgui_render_time = 0.0f;

namespace iara {

    extern const std::filesystem::path g_assets_path;

    namespace color {
        static glm::vec4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
        static glm::vec4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
        static glm::vec4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
    }

    EditorLayer::EditorLayer()
    {
    }

    void EditorLayer::onAttach() {
        m_active_scene = CreateRef<Scene>();
        FramebufferSpecification specs = m_active_scene->getMainFramebuffer()->getSpecification();
        m_viewportSize.x = (uint32_t)specs.width;
        m_viewportSize.y = (uint32_t)specs.height;

        m_editor_camera = EditorCamera(60.0f, 1.778f, 0.01f, 10000.0f);
        
        m_scene_h_panel.setContext(m_active_scene);

        m_stop_icon = Texture2D::Create("Assets\\Textures\\stop3.png");
        m_play_icon = Texture2D::Create("Assets\\Textures\\play.png");
        m_active_scene->setSkyBox("Assets\\Textures\\skybox2\\sky1.png");
        }

    void EditorLayer::onDetach() {

    }

    void EditorLayer::onUpdate(iara::Timestep ts) {
        Timer timer;
        m_frame_interval_acc = ts;
        //IARA_CORE_INFO("FRAME INTERVAL ACCUMULATOR : {0}", m_frame_interval_acc);

        /// Resize
        if (FramebufferSpecification spec = m_active_scene->getMainFramebuffer()->getSpecification();
            m_viewportSize.x > 0.0f && m_viewportSize.y > 0.0f &&
            (spec.width != m_viewportSize.x || spec.height != m_viewportSize.y)) {
            
            m_active_scene->getMainFramebuffer()->resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
            m_editor_camera.setViewportSize(m_viewportSize.x, m_viewportSize.y);
            m_active_scene->onViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
        }
  
        /// Render
        if (m_active_scene->getSceneState() == SceneState::EDIT) {
            m_editor_camera.onUpdate(ts);
            m_active_scene->onUpdateEditor(ts, m_editor_camera);
        }
        else {
            m_active_scene->onUpdateRuntime(ts);
        }


        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_viewport_bounds[0].x;
        my -= m_viewport_bounds[0].y;
        glm::vec2 viewport_size = m_viewport_bounds[1] - m_viewport_bounds[0];
        my = viewport_size.y - my;

        int mousex = (int)mx;
        int mousey = (int)my;

        if (mousex >= 0 && mousey >= 0 && mousex <= (int)viewport_size.x && mousey <= (int)viewport_size.y) {
            m_hovered_pixel_entity = m_active_scene->getMainFramebuffer()->readPixel(1, mousex, mousey);
        }
        //IARA_CORE_TRACE("Hovored pixel: {0}", m_hovered_pixel_entity);
        g_on_update_time = timer.elapsedMilliseconds();
    }

    void EditorLayer::onImGuiRender() {

        bool dockspace = true;

        if (dockspace) {
            Timer timer;
            onImGuiRenderBeginDocking();

            onImGuiRenderMenuBar();
            onImGuiRenderSettings();

            m_scene_h_panel.onImGuiRender();
            m_browser_panel.onImGuiRender();

            onImGuiRenderViewport();
            onImGuiRenderActionBar();
            onImGuiRenderEnvironment();

            ///ImGui::ShowDemoWindow();
            g_on_imgui_render_time = timer.elapsedMilliseconds();
            ImGui::Begin("##asdddd");

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("FrameInterval %f", m_frame_interval_acc * 1000 );
            ImGui::Text("EditorLayer::OnImGuiRender %f", g_on_imgui_render_time);
            ImGui::Text("EditorLayer::onUpdate %f", g_on_update_time);
            ImGui::Text("ShadowMap Render %f", m_active_scene->render_shadowmap_timer);
            ImGui::End();
            ImGui::End();
        }
    }

    void EditorLayer::onEvent(iara::Event& event) {
        m_editor_camera.onEvent(event);

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(IARA_BIND_EVENT_FN(EditorLayer::onKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(IARA_BIND_EVENT_FN(EditorLayer::onMouseButtonPressed));
    }

    bool EditorLayer::onKeyPressed(KeyPressedEvent& e) {
        /// Shortcuts
        if (e.getRepeatCount() > 0) return false;

        bool controll = Input::IsKeyPressed(IARA_KEY_LEFT_CONTROL);
        bool shiftt = Input::IsKeyPressed(IARA_KEY_LEFT_SHIFT);
        switch (e.getKeyCode())
        {
            case IARA_KEY_S: 
                if (controll && shiftt) {
                    SaveSceneAs();
                }
                break;
            
            case IARA_KEY_N: 
                if (controll) {
                    NewScene();
                }
                break;
            
            case IARA_KEY_O: 
                if (controll) {
                    OpenScene();
                }
                break;

            /*case IARA_KEY_S:
                m_editor_camera.moveDown();*/
           

            /// Gizmo
            case IARA_KEY_Q:
                m_gizmo_type = -1;
                break;
            case IARA_KEY_W:
                m_gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
                break;
            case IARA_KEY_E:
                m_gizmo_type = ImGuizmo::OPERATION::ROTATE;
                break;
            case IARA_KEY_R:
                m_gizmo_type = ImGuizmo::OPERATION::SCALE;
                break;
        }
        return false;
    }

    bool EditorLayer::onMouseButtonPressed(MouseButtonPressedEvent& e) {
        bool shiftt = Input::IsKeyPressed(IARA_KEY_LEFT_SHIFT);
        if (shiftt && Input::IsMouseButtonPressed(IARA_MOUSE_BUTTON_LEFT)) {
            m_scene_h_panel.setMouseHovered((uint32_t)m_hovered_pixel_entity);
            m_selected_entity = m_scene_h_panel.getSelectedEntity();
        }
        return false;
    }

    void EditorLayer::NewScene() {
        m_active_scene = CreateRef<Scene>();
        m_active_scene->onViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
        m_scene_h_panel.setContext(m_active_scene);

        m_selected_entity = {};
    }

    void EditorLayer::OpenScene() {
        std::string filepath = FileDialogs::openFile("IARA Scene (*.iara)\0*.iara\0");
        if (!filepath.empty()) {
            OpenScene(std::filesystem::path(filepath));
        }
    }

    void EditorLayer::OpenScene(std::filesystem::path& path) {
        m_active_scene = CreateRef<Scene>();
        SceneSerializer serializer(m_active_scene);
        serializer.deserialize(path.string());

        m_active_scene->onViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
        //s_scene_renderer.setScene(m_active_scene);
        m_scene_h_panel.setContext(m_active_scene);
        m_selected_entity = {};
    }

    void EditorLayer::SaveSceneAs() {
        std::string filepath = FileDialogs::saveFile("IARA Scene (*.iara)\0*.iara\0");
        if (!filepath.empty()) {
            SceneSerializer serializer(m_active_scene);
            serializer.serialize(filepath);
        }
    }

    void EditorLayer::onImGuiRenderBeginDocking() {
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
    }

    void EditorLayer::onImGuiRenderMenuBar() {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.

                if (ImGui::MenuItem("New", "Ctrl+N")) {
                    NewScene();
                }
                else if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                    OpenScene();
                }
                else if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                    SaveSceneAs();
                }

                if (ImGui::MenuItem("Exit")) iara::Application::Get().Close();
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void EditorLayer::onImGuiRenderSettings() {
        ImGui::Begin("Settings");

        auto stats = iara::Renderer2D::getStats();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("EditorLayer::onUpdate %f", g_on_update_time);
        ImGui::Text("Renderer Stats: ");
        ImGui::Text("Draw Calls: %d", stats.draw_calls);
        ImGui::Text("Quads: %d", stats.quad_count);
        ImGui::Text("Total Vertices: %d", stats.GetVertices());
        ImGui::Text("Total Indices: %d", stats.GetIndices());
        

        //IARA_CORE_TRACE("ShadowMapQuad Color Attachment ID: {}", m_active_scene->getShadowMapQuad()->getColorAtt(0));
        uint32_t texID = m_active_scene->getShadowMapQuad()->getColorAtt(0);  /// Get the texture from the framebuffer
        ImGui::Image((void*)(intptr_t)texID, ImVec2(300, 300), ImVec2{ 0,1 }, ImVec2{ 1,0 });

        ImGui::End();
    }

    void EditorLayer::onImGuiRenderViewport() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

        ImGui::Begin("Viewport");
        auto viewport_min_region = ImGui::GetWindowContentRegionMin();
        auto viewport_max_region = ImGui::GetWindowContentRegionMax();
        auto viewport_offset = ImGui::GetWindowPos(); // includes the tab bar

        m_viewport_bounds[0] = { viewport_min_region.x + viewport_offset.x, viewport_min_region.y + viewport_offset.y };
        m_viewport_bounds[1] = { viewport_max_region.x + viewport_offset.x, viewport_max_region.y + viewport_offset.y };

        m_viewportFocus = ImGui::IsWindowFocused();
        m_viewportHover = ImGui::IsWindowHovered();
        Application::Get().getImguiLayer()->BlockEvents(!m_viewportFocus && !m_viewportHover);

        ImVec2 viewportPaneSize = ImGui::GetContentRegionAvail();
        if (m_viewportSize.x != viewportPaneSize.x || m_viewportSize.y != viewportPaneSize.y) {
            //m_framebuffer->resize((uint32_t)viewportPaneSize.x, (uint32_t)viewportPaneSize.y);
            m_viewportSize = { viewportPaneSize.x , viewportPaneSize.y };
        }
        //ZIARA_INFO("Viewport size: {0} x {1}", viewportPaneSize.x, viewportPaneSize.y);
        uint32_t texID = m_active_scene->getMainFramebuffer()->getColorAtt(0);  /// Get the texture from the framebuffer
        ImGui::Image((void*)(intptr_t)texID, ImVec2(m_viewportSize.x, m_viewportSize.y), ImVec2{ 0,1 }, ImVec2{ 1,0 });

        if (ImGui::BeginDragDropTarget()) {
            /// this payload can be null, that's why it's going through a check
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("content_browser_item")) {
                const wchar_t* path = (const wchar_t*)payload->Data;
                std::string iara = std::filesystem::path(path).extension().string();
                if (iara == ".iara")
                    OpenScene(g_assets_path / std::filesystem::path(path));
            }
            ImGui::EndDragDropTarget();
        }

        Entity selected_entity;
        /// Gizmo

        m_selected_entity = m_scene_h_panel.getSelectedEntity();

        if (m_selected_entity && m_gizmo_type != -1 && !m_selected_entity.hasComponent<DirLightComponent>() && m_active_scene->getSceneState() == SceneState::EDIT) {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(m_viewport_bounds[0].x, m_viewport_bounds[0].y, m_viewport_bounds[1].x - m_viewport_bounds[0].x, m_viewport_bounds[1].y - m_viewport_bounds[0].y);

            /// Camera Runtime0
                /// auto camera_entity = m_active_scene->getPrimaryCameraEntity();
                /// auto& camera = camera_entity.getComponent<CameraComponent>().camera;
                /// const glm::mat4& camera_proj = camera.getProjection();
                /// glm::mat4 camera_view = glm::inverse(camera_entity.getComponent<TransformComponent>().getTransform());


            /// Editor Camera
            const glm::mat4& camera_proj = m_editor_camera.getProjection();
            glm::mat4 camera_view = m_editor_camera.getViewMatrix();

            /// Entity transform 
            auto& tc = m_selected_entity.getComponent<TransformComponent>();
            glm::mat4 transform = tc.getTransform();

            /// Snapping
            bool snap = Input::IsKeyPressed(IARA_KEY_LEFT_CONTROL);
            float snap_val = 0.5f; /// snap to 0.5m for translation and scale
            if (m_gizmo_type == ImGuizmo::OPERATION::ROTATE)
                snap_val = 45.0f;

            float snap_vals[3] = { snap_val, snap_val, snap_val };

            ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(camera_proj),
                ImGuizmo::OPERATION(m_gizmo_type), ImGuizmo::LOCAL, glm::value_ptr(transform),
                nullptr, snap ? snap_vals : nullptr);

            if (ImGuizmo::IsUsing()) {
                glm::vec3 translation, rotation, scale;
                math::decomposeTransform(transform, translation, rotation, scale);


                glm::vec3 delta_rotation = rotation - tc.rotation;
                tc.translation = translation;
                tc.rotation += delta_rotation;
                tc.scale = scale;
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void EditorLayer::onImGuiRenderActionBar() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        ImGui::Begin("##Action_bar", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDecoration);

        float size = ImGui::GetWindowHeight() - 4.0f;

        Ref<Texture2D> button_tex = m_active_scene->getSceneState() == SceneState::EDIT ? m_play_icon : m_stop_icon;
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x * 0.5f - size * 0.5f);
        if (ImGui::ImageButton("play button", (ImTextureID)button_tex->getRendererID(), ImVec2(size + 6.0f, size))) {
            if (m_active_scene->getSceneState() == SceneState::EDIT) {
                m_active_scene->setSceneState(SceneState::PLAY);
            }
            else {
                m_active_scene->setSceneState(SceneState::EDIT);
            }
        }

        ImGui::End();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    void EditorLayer::onImGuiRenderEnvironment() {
        ImGui::Begin("Environment");

        ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
        if (ImGui::BeginDragDropTarget()) {
            /// this payload can be null, that's why it's going through a check
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("content_browser_item")) {
                const wchar_t* path = (const wchar_t*)payload->Data;
                std::string iara = std::filesystem::path(path).extension().string();

                std::filesystem::path tex_path = g_assets_path / std::filesystem::path(path);
                if (iara == ".png") {
                    m_active_scene->setSkyBox(tex_path.string());
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::Checkbox("PolygonMode", &m_polygonMode)) {
            RenderCommand::polygonMode(m_polygonMode);
        }

        ImGui::End();
    }
}