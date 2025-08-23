#pragma once

#include "entt.hpp"

#include "iara/Core/Timestep.h"
#include "iara\Renderer\EditorCamera.h"
#include "iara\Renderer\Texture.h"
#include "iara/Renderer/Framebuffer.h"

namespace iara {

	class Entity;

	enum class RenderingType : uint8_t {
		None = 0,
		MSAA,
		HDR,
		DEFERRED
	};

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

		Entity getPrimaryCameraEntity();
		uint32_t getPointLights() const { return m_plights; }
		void decreasePointLights() { m_plights--; }
		void increasePointLights() { m_plights++; }

		void setSkyBox(const std::string& path) { m_skybox = Texture2D::CreateCubemap(path); m_skybox_path = path; }


		bool getDirLight() { return m_dlight; }
		void setDirLight() { m_dlight = true; }
		void clearDirLight() { m_dlight = false; }

		inline const Ref<Framebuffer> getShadowMap() const { return m_shadow_map; }
		inline const Ref<Framebuffer> getShadowMapQuad() const { return m_shadowmap_quad; }
		inline const Ref<Framebuffer> getMainFramebuffer() const { return m_main_framebuffer; }
		inline const Ref<Framebuffer> getFinalFramebuffer() const { return m_final_framebuffer; }
		inline const Ref<Framebuffer> getFinal2Framebuffer() const { return m_final2_framebuffer; }
		inline const Ref<Framebuffer> getSSAOFramebuffer() const { return m_ssao_framebuffer; }

		void initializeFramebuffers();
		const uint32_t getFinalRenderedTexture();

		inline void setSceneState(SceneState state) { m_scene_state = state; }
		inline SceneState getSceneState() { return m_scene_state; }

		RenderingType rendering_type = RenderingType::DEFERRED;

		float render_shadowmap_timer = 0.0f;
	private:
		template<typename T>
		void onComponentAdded(Entity entity, T& component);

		void applyToneMapping(EditorCamera& camera, uint32_t hdr_texture);

		void initializeShadowMap();

		/// RENDER PASSES
		void render2DPassEdit(EditorCamera& camera);
		void renderToShadowMapPass(const glm::mat4& light_vp);
		void render3DPassEdit(EditorCamera& camera, const glm::mat4& light_vp);

		void render2DPassRuntime(Camera& camera, const glm::mat4& camera_transform);
		void render3DPassRuntime(Camera& camera, const glm::mat4& camera_transform, const glm::mat4& light_vp);

		void renderShadowMapToColorFBO();
		/// -------------
	private:
		entt::registry m_registry;

		std::string m_skybox_path;
		Ref<Texture2D> m_skybox = nullptr;

		Ref<Framebuffer> m_shadow_map = nullptr;
		Ref<Framebuffer> m_shadowmap_quad = nullptr;

		Ref<Framebuffer> m_main_framebuffer = nullptr;   // used for MSAA or HDR or the G-Buffer
		Ref<Framebuffer> m_final_framebuffer = nullptr;  // used for tonemapping in case of HDR or the final fbo for deferred rendering 
		Ref<Framebuffer> m_final2_framebuffer = nullptr; // TODO: CHANGE THESE NAMES
		Ref<Framebuffer> m_ssao_framebuffer = nullptr;


		glm::mat4 cascade1;
		uint32_t m_vp_width = 1;
		uint32_t m_vp_height = 1;

		uint32_t m_plights = 0;
		bool m_dlight = false;

		SceneState m_scene_state = SceneState::EDIT;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}