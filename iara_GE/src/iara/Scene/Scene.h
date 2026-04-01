#pragma once

#include "entt.hpp"

#include "iara/Core/Timestep.h"
#include "iara\Renderer\EditorCamera.h"
#include "iara\Renderer\Texture.h"
#include "iara/Renderer/Framebuffer.h"
#include "iara/Atmosphere/Atmosphere.h"
#include "iara/Atmosphere/atmosphere2.h"

namespace iara {

	class Entity;

	enum class RenderingType : uint8_t {
		None = 0,
		MSAA,
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
		void onUpdateEditor(float deltaTime, EditorCamera& camera, glm::vec2 mouse_pos);
		void onViewportResize(uint32_t width, uint32_t height);

		void onSunMovedUpdate();

		void resizeFramebuffers(uint32_t width, uint32_t height);

		Entity getPrimaryCameraEntity();
		uint32_t getPointLights() const { return m_plights; }
		void decreasePointLights() { m_plights--; }
		void increasePointLights() { m_plights++; }

		void setSkyBox(const std::string& path) { m_skybox = Texture2D::CreateCubemap(path); m_skybox_path = path; }

		bool getDirLight() { return m_dlight; }
		void setDirLight() { m_dlight = true; }
		void clearDirLight() { m_dlight = false; }

		inline const Ref<Framebuffer> getShadowMap() const { return m_shadow_map_cascade0; }
		inline const Ref<Framebuffer> getShadowMapQuad() const { return m_shadowmap_quad; }
		inline const Ref<Framebuffer> getMSAAFramebuffer() const { return m_msaa_framebuffer; }
		inline const Ref<Framebuffer> getGBufferFramebuffer() const { return m_gbuffer_framebuffer; }
		inline const Ref<Framebuffer> getDeferredLightingFramebuffer() const { return m_deferred_final_ldr_framebuffer; }
		inline const Ref<Framebuffer> getDeferredHDRFramebuffer() const { return m_deferred_hdr_framebuffer; }
		inline const Ref<Framebuffer> getSSAOFramebuffer() const { return m_ssao_framebuffer; }
		inline const Ref<Framebuffer> getSSAOBlurFramebuffer() const { return m_ssao_blur_framebuffer; }
		inline const Ref<Framebuffer> getAtmFramebuffer() const { return m_atm_fbo; }

		inline const Ref<Atmosphere>  getAtmosphere() { return m_atmosphere; }

		void initializeFramebuffers();
		const uint32_t getFinalRenderedTexture();

		inline void setSceneState(SceneState state) { m_scene_state = state; }
		inline SceneState getSceneState() { return m_scene_state; }

		RenderingType rendering_type = RenderingType::DEFERRED;

		float render_shadowmap_timer = 0.0f;
		float shadow_map_radius = 10.0f;
		float shadow_map_light_distance = 20.0f;
		glm::mat4 m_shadowmap_projection;
		bool use_ssao = true;

		glm::vec3 sun_direction = glm::vec3(-0.5f, 0.5f, -0.3f);
		//float m_sun_angular_radius = 0.00935 / 2.0;
		float m_sun_angular_radius = 0.01;
		float m_view_distance = 9000.0;
		float m_view_zenith_angle_radians_ = 1.47;
		float m_view_azimuth_angle_radians_ = 0.0;
	private:
		template<typename T>
		void onComponentAdded(Entity entity, T& component);

		void applyToneMapping(float exposure, uint32_t hdr_texture);

		void initializeShadowMap();
		void computeLightSpaceMatrices(const EditorCamera& camera);
		glm::mat4 computeCascadeMatrix(float near_clip, float far_clip, const EditorCamera& camera);
		std::vector<glm::vec4> computeFrostumCornersWS(const glm::mat4& camera);

		void initializeAtmosphere();

		/// RENDER PASSES
		void renderAtmosphere(EditorCamera& camera);
		void depthPassAtmosphere(EditorCamera& camera);

		void renderShadowmapCascadesPass();
		void renderToShadowMapPass(const glm::mat4& light_vp);
		void render2DPassEdit(EditorCamera& camera);
		void render3DPassEdit(EditorCamera& camera, const glm::mat4& light_vp);

		void render2DPassRuntime(Camera& camera, const glm::mat4& camera_transform);
		void render3DPassRuntime(Camera& camera, const glm::mat4& camera_transform, const glm::mat4& light_vp);

		void renderShadowMapToColorFBO();
		void renderCSMToColorFBO();
		/// -------------
	private:
		entt::registry m_registry;

		std::string m_skybox_path;
		Ref<Texture2D> m_skybox = nullptr;
		Ref<Atmosphere> m_atmosphere = nullptr;

		Ref<Framebuffer> m_shadow_map_cascade0 = nullptr;
		Ref<Framebuffer> m_shadow_map_cascade1 = nullptr;
		Ref<Framebuffer> m_shadow_map_cascade2 = nullptr;
		Ref<Framebuffer> m_shadow_map_cascade3 = nullptr;

		Ref<Framebuffer> m_shadowmap_quad = nullptr;

		Ref<Framebuffer> m_msaa_framebuffer = nullptr;   // used for MSAA 
		Ref<Framebuffer> m_gbuffer_framebuffer = nullptr; // used for deferred rendering
		Ref<Framebuffer> m_deferred_final_ldr_framebuffer = nullptr;  // used for tonemapping in case of HDR or the final fbo for deferred rendering 
		Ref<Framebuffer> m_deferred_hdr_framebuffer = nullptr; // TODO: CHANGE THESE NAMES
		Ref<Framebuffer> m_deferred_atmosphere_framebuffer = nullptr; // this will contain the geometry from lighting pass and the background sky
		Ref<Framebuffer> m_ssao_framebuffer = nullptr;
		Ref<Framebuffer> m_ssao_blur_framebuffer = nullptr;

		Ref<Framebuffer> m_atm_fbo = nullptr;

		std::vector<float> m_shadow_cascade_levels;
		std::vector<glm::mat4> m_cascades;
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