#pragma once

#include "iara\Scene\Scene.h"
#include "iara\Scene\Component.h"
#include "iara\Scene\Entity.h"

namespace iara {

	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void setContext(const Ref<Scene>& scene);

		void onImGuiRender();

		Entity getSelectedEntity() const { return m_selection_context; }

		Entity setMouseHovered(uint32_t entityID);
	private:
		void drawEntityNode(Entity entity);
		void drawComponents(Entity entity);
	private:
		Ref<Scene> m_context;
		Entity m_selection_context;

		Ref<Texture2D> m_texture_placeholder = Texture2D::Create("Assets\\Textures\\check2.png");

	};

}