#pragma once

#include "iara.h"
#include "Panels\SceneHierarchyPanel.h"
#include "Panels\ContentBrowserPanel.h"

#include "iara\Scene\SceneRenderer.h"

namespace iara {

	static SceneRenderer s_scene_renderer;

	class EditorLayer : public iara::Layer {
	public:
		EditorLayer();
		virtual void onAttach() override;
		virtual void onDetach() override;
		virtual void onUpdate(iara::Timestep ts) override;
		virtual void onImGuiRender() override;
		virtual void onEvent(iara::Event& event) override;
	private:
		bool onKeyPressed(KeyPressedEvent& e);
		bool onMouseButtonPressed(MouseButtonPressedEvent& e);

		void onImGuiRenderMenuBar();
		void onImGuiRenderSettings();
		void onImGuiRenderMaterialCP();
		void onImGuiRenderViewport();
		void onImGuiRenderActionBar();
		void onImGuiRenderBeginDocking();

		void NewScene();
		void OpenScene();
		void OpenScene(std::filesystem::path& path);
		void SaveSceneAs();
	private:
		iara::Ref<iara::Framebuffer> m_framebuffer;
		glm::vec2 m_viewportSize;
		glm::vec2 m_viewport_bounds[2];
		bool m_viewportFocus = false, m_viewportHover = false;
		
		Ref<Scene> m_active_scene;
		Ref<Scene> m_editor_scene;

		EditorCamera m_editor_camera;
		/// Panel
		SceneHierarchyPanel m_scene_h_panel;
		ContentBrowserPanel m_browser_panel;

		int m_gizmo_type = -1;
		int m_hovered_pixel_entity = -1;
		Entity m_selected_entity;
		Material m_editable_material = {};
		Material m_material_add = {};
		int m_selected_material = 0;

		bool once = true;

		Ref<Texture2D> m_play_icon;
		Ref<Texture2D> m_stop_icon;
	};
}