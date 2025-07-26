#pragma once

#include "entt.hpp"

#include "iara/Core/Timestep.h"
#include "iara\Renderer\EditorCamera.h"
#include "iara\Renderer\Texture.h"
#include "iara/Renderer/Framebuffer.h"

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
		Entity createMeshObject(const std::string& name = std::string());
		void destroyEntity(Entity entity);
		const bool validEntity(Entity ent);

		void onUpdateRuntime(Timestep ts);
		void onUpdateEditor(Timestep ts, EditorCamera& camera);
		void onViewportResize(uint32_t width, uint32_t height);

		/// RENDER PASSES
		void render2DPassEdit(EditorCamera& camera);
		void renderToShadowMapPass(const glm::mat4& light_vp);
		void render3DPassEdit(EditorCamera& camera, const glm::mat4& light_vp);

		void render2DPassRuntime(Camera& camera, const glm::mat4& camera_transform);
		void render3DPassRuntime(Camera& camera, const glm::mat4& camera_transform, const glm::mat4& light_vp);

		void renderShadowMapToColorFBO();
		/// -------------

		Entity getPrimaryCameraEntity();
		uint32_t getPointLights() const { return m_plights; }
		void decreasePointLights() { m_plights--; }
		void increasePointLights() { m_plights++; }

		void setSkyBox(const std::string& path) { m_skybox = Texture2D::CreateCubemap(path); m_skybox_path = path; }
		void initializeShadowMap();
		void initializeMainFramebuffer();

		bool getDirLight() { return m_dlight; }
		void setDirLight() { m_dlight = true; }
		void clearDirLight() { m_dlight = false; }

		const Ref<Framebuffer> getShadowMap() const { return m_shadow_map; }
		const Ref<Framebuffer> getShadowMapQuad() const { return m_shadowmap_quad; }
		const Ref<Framebuffer> getMainFramebuffer() const { return m_main_framebuffer; }

		void setSceneState(SceneState state) { m_scene_state = state; }
		SceneState getSceneState() { return m_scene_state; }

		float render_shadowmap_timer = 0.0f;
	private:
		template<typename T>
		void onComponentAdded(Entity entity, T& component);
	private:
		friend class Entity;
		uint32_t m_vp_width  = 1;
		uint32_t m_vp_height = 1;


		uint32_t m_plights = 0;
		bool m_dlight = false;
		std::string m_skybox_path;
		Ref<Texture2D> m_skybox = nullptr;

		Ref<Framebuffer> m_shadow_map = nullptr;
		Ref<Framebuffer> m_main_framebuffer = nullptr;
		Ref<Framebuffer> m_shadowmap_quad = nullptr;

		glm::mat4 cascade1;

		entt::registry m_registry;

		SceneState m_scene_state = SceneState::EDIT;
		
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}