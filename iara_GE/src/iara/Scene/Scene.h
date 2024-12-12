#pragma once

#include "entt.hpp"

#include "iara/Core/Timestep.h"
#include "iara\Renderer\EditorCamera.h"

namespace iara {

	class Entity;

	enum class SceneState {
		EDIT = 0,
		PLAY = 1
	};

	class Scene {
	public:
		Scene();
		~Scene();

		Entity createEntity(const std::string& name = std::string());
		Entity createPointLight(const std::string& name = std::string());
		Entity createDirLight(const std::string& name = std::string());
		void destroyEntity(Entity entity);

		void onUpdateRuntime(Timestep ts);
		void onUpdateEditor(Timestep ts, EditorCamera& camera);
		void onViewportResize(uint32_t width, uint32_t height);

		Entity getPrimaryCameraEntity();
		uint32_t getPointLights() const { return m_plights; }
		void decreasePointLights() { m_plights--; }
		void increasePointLights() { m_plights++; }

		void setSceneState(SceneState state) { m_scene_state = state; }
		SceneState getSceneState() { return m_scene_state; }
	private:
		template<typename T>
		void onComponentAdded(Entity entity, T& component);
	private:
		friend class Entity;
		uint32_t m_vp_width  = 1;
		uint32_t m_vp_height = 1;

		uint32_t m_plights = 0;

		entt::registry m_registry;

		SceneState m_scene_state = SceneState::EDIT;
		
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}