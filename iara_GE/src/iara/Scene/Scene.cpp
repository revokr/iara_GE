#include "ir_pch.h"
#include "Scene.h"

#include "Component.h"
#include "Entity.h"
#include "../Core/Timer.h"
#include "iara/Renderer/Renderer.h"
#include <glm/ext/matrix_transform.hpp>
#include <glad\glad.h>

#include <stb_image_write.h>


#include "iara\Math\Math.h"


namespace iara {

	Scene::Scene() {
		m_registry = {};

		initializeAtmosphere();

		initializeFramebuffers();
		initializeShadowMap();
	}

	Scene::~Scene() { }

	Entity Scene::createEntity(const std::string& name) {
		Entity entity{ m_registry.create(), this };
		entity.addComponent<TransformComponent>();
		auto& tag = entity.addComponent<TagComponent>();
		tag.tag = name.empty() ? "Unnamed_EnTiTy" : name;

		return entity;
	}

	Entity Scene::createPointLight(const std::string& name) {
		Entity entity{ m_registry.create(), this };
		entity.addComponent<TransformComponent>();
		auto& tag = entity.addComponent<TagComponent>();
		tag.tag = name.empty() ? "Point Light" : name;

		entity.addComponent<PointLightComponent>();
		m_plights++;

		return entity;
	}

	Entity Scene::createDirLight(const std::string& name) {
		Entity entity{ m_registry.create(), this };
		auto& tag = entity.addComponent<TagComponent>();
		tag.tag = name.empty() ? "Directional Light" : name;

		entity.addComponent<DirLightComponent>();
		m_dlight = true;
		return entity;
	}

	Entity Scene::createMeshObject(const std::string& name) {
		Entity entity{ m_registry.create(), this };
		auto& tag = entity.addComponent<TagComponent>();
		tag.tag = name.empty() ? "Mesh Object" : name;

		entity.addComponent<TransformComponent>();
		entity.addComponent<MeshComponent>();

		return entity;
	}

	void Scene::destroyEntity(Entity entity) {
		m_registry.destroy(entity);
	}

	const bool Scene::validEntity(Entity ent) {
		return m_registry.valid(ent);
	}

	void Scene::onUpdateRuntime(Timestep ts) {
		/*/// Update Scripts
		{
			auto view = m_registry.view<NativeScriptComponent>();
			for (auto entity : view) {
				auto& nsc = view.get<NativeScriptComponent>(entity);

				if (!nsc.instance) {
					nsc.instance = nsc.instantiateScript();
					nsc.instance->m_entity = Entity{ entity, this };
					nsc.instance->onCreate();
				}

				nsc.instance->onUpdate(ts);
			}
		}*/

		Camera* main_camera = nullptr;
		glm::mat4 camera_transform;
		{
			auto view = m_registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view) {
				auto [transform, camera] = view.get< TransformComponent, CameraComponent>(entity);

				if (camera.primary) {
					main_camera = &camera.camera;
					camera_transform = transform.getTransform();
					break;
				}
			}
		}

		if (main_camera) {
			/*
			/// Upload Meshes
			auto view_mesh = m_registry.view<TransformComponent, MeshComponent>();
			for (auto entity : view_mesh) {
				auto [transf, mesh] = view_mesh.get<TransformComponent, MeshComponent>(entity);
				MeshRenderer::drawMesh(transf.getTransform(), mesh, (int)entity);
			}

			//Timer timer;
			renderToShadowMapPass(cascade1);
			renderShadowMapToColorFBO();

			if (rendering_type == RenderingType::MSAA) {
				// MSAA Forward Pass
				//render_shadowmap_timer = timer.elapsedMilliseconds();
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				m_msaa_framebuffer->bind();
				iara::Renderer2D::ResetStats();
				iara::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.5f, 1.0f });
				iara::RenderCommand::Clear();
				m_msaa_framebuffer->clearAttachment(1, -1);

				if (m_skybox) {
					glm::mat4 view3 = glm::mat4(glm::mat3(glm::inverse(camera_transform)));
					Renderer3D::drawSkyBox(main_camera->getProjection() * view3, m_skybox, sun_direction);
				}

				render2DPassRuntime(*main_camera, camera_transform);
				render3DPassRuntime(*main_camera, camera_transform, cascade1);
				m_msaa_framebuffer->unbind();
			}
			else if (rendering_type == RenderingType::DEFERRED) {
				///GEOMETRY PASS
				/// SKYBOX RENDERS ON TOP OF EVERYTHING --- FIXED USING ENTITY ID AND PASSING THAT TO THE
				///									   LIGHTINGSHADER WHERE IT TESTS IF THE PIXEL HAS ENTITYID > 10000 (arbitrary)
				glDisable(GL_BLEND);

				m_gbuffer_framebuffer->bind();
				iara::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
				iara::RenderCommand::Clear();
				m_main_framebuffer->clearAttachment(0, -1);
				m_main_framebuffer->clearAttachment(1, -1);
				m_main_framebuffer->clearAttachment(2, -1);
				m_gbuffer_framebuffer->clearAttachment(3, -1);

				if (m_skybox) {
					glm::mat4 view3 = glm::mat4(glm::mat3(glm::inverse(camera_transform)));
					Renderer3D::drawSkyBox(main_camera->getProjection() * view3, m_skybox, sun_direction);
				}

				Renderer2D::BeginScene(*main_camera, camera_transform, m_plights, m_dlight);
				auto view4 = m_registry.view<TransformComponent, PointLightComponent>();
				for (auto entity : view4) {
					auto [transf, plight] = view4.get<TransformComponent, PointLightComponent>(entity);
					Renderer2D::drawLight(transf.getTransform(), plight, *main_camera, (int)entity);
				}

				auto view5 = m_registry.view<DirLightComponent>();
				auto entity = view5.front();
				for (auto entity : view5) {
					auto dlight = view5.get<DirLightComponent>(entity);
					Renderer2D::drawDirLight(dlight);
				}
				Renderer2D::EndScene();


				MeshRenderer::GeometryPassGBuffer(*main_camera, camera_transform);
				m_gbuffer_framebuffer->unbind();

				/// SSAO PASS
				m_ssao_framebuffer->bind();
				RenderCommand::ClearColorBuffer();
				MeshRenderer::SSAOPass(*main_camera, camera_transform, m_vp_width, m_vp_height, m_gbuffer_framebuffer->getColorAtt(0), m_gbuffer_framebuffer->getColorAtt(1), m_gbuffer_framebuffer->getColorAtt(3));
				m_ssao_framebuffer->unbind();

				m_ssao_blur_framebuffer->bind();
				RenderCommand::ClearColorBuffer();
				MeshRenderer::SSAOBlurPass(m_ssao_framebuffer->getColorAtt(0));
				m_ssao_blur_framebuffer->unbind();

				/// LIGHTING PASS
				m_deferred_hdr_framebuffer->bind();
				RenderCommand::Clear();
				MeshRenderer::LighintgPass(*main_camera, camera_transform, m_gbuffer_framebuffer->getColorAtt(0), m_gbuffer_framebuffer->getColorAtt(1), m_gbuffer_framebuffer->getColorAtt(2), m_gbuffer_framebuffer->getColorAtt(3), m_shadow_map->getDepthAtt(), m_ssao_blur_framebuffer->getColorAtt(0), cascade1, use_ssao);
				m_deferred_hdr_framebuffer->unbind();

				applyToneMapping(main_camera->getExposure(), m_deferred_hdr_framebuffer->getColorAtt(0));
			}
			*/

			//MeshRenderer::ResetSceneMeshes();

		}
	}

	void Scene::onUpdateEditor(float deltaTime, EditorCamera& camera, glm::vec2 mouse_pos) {
		auto view_mesh = m_registry.view<TransformComponent, MeshComponent>();
		for (auto entity : view_mesh) {
			auto [transf, mesh] = view_mesh.get<TransformComponent, MeshComponent>(entity);
			MeshRenderer::drawMesh(transf.getTransform(), mesh, (int)entity);
		}

		Timer timer;
		
		glm::vec3 to_sun = glm::normalize(glm::vec3(sun_direction.x, sun_direction.y, sun_direction.z));
		glm::vec3 target = camera.getPosition();
		glm::vec3 light_position = target + to_sun * shadow_map_light_distance;
		glm::vec3 up_vector = glm::vec3(0.0f, 1.0f, 0.0f);
		float r = shadow_map_radius;
		glm::mat4 lightProjection = glm::ortho(-r, r, -r, r, 1.0f, 100.0f);
		glm::mat4 lightView = glm::lookAt(light_position, target, up_vector);

		cascade1 = lightProjection * lightView;
		//renderToShadowMapPass(cascade1);
		//renderShadowMapToColorFBO();


		computeLightSpaceMatrices(camera);
		renderShadowmapCascadesPass();
		renderShadowMapToColorFBO();
		/// Wrong Cascade matrix calculation most likely - need to fix
		

		if (rendering_type == RenderingType::MSAA) {
			// MSAA Forward Pass
			render_shadowmap_timer = timer.elapsedMilliseconds();
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			m_msaa_framebuffer->bind();
			iara::Renderer2D::ResetStats();
			//iara::RenderCommand::SetClearColor({ 0.8f, 0.2f, 0.5f, 1.0f });
			iara::RenderCommand::Clear();
			m_msaa_framebuffer->clearAttachment(1, -1);

			if (m_skybox) {
				glm::mat4 view3 = glm::mat4(glm::mat3(camera.getViewMatrix()));
				Renderer3D::drawSkyBox(camera.getProjection() * view3, m_skybox, glm::vec4(to_sun.x, to_sun.y, to_sun.z, 1.0));
			}
			glDisable(GL_DEPTH_TEST);
			//renderAtmosphere(camera);
			glEnable(GL_DEPTH_TEST);
			render2DPassEdit(camera);
			render3DPassEdit(camera, cascade1);
			m_msaa_framebuffer->unbind();
		}
		else if (rendering_type == RenderingType::DEFERRED) {
			

			glDisable(GL_BLEND);

			m_gbuffer_framebuffer->bind();
			iara::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
			iara::RenderCommand::Clear();
			m_gbuffer_framebuffer->clearAttachment(0, -1);
			m_gbuffer_framebuffer->clearAttachment(1, -1);
			m_gbuffer_framebuffer->clearAttachment(2, -1);
			m_gbuffer_framebuffer->clearAttachment(3, -1);
			m_gbuffer_framebuffer->clearAttachment(4, -1);

			Renderer2D::BeginScene(camera, m_plights, m_dlight);
			auto view4 = m_registry.view<TransformComponent, PointLightComponent>();
			for (auto entity : view4) {
				auto [transf, plight] = view4.get<TransformComponent, PointLightComponent>(entity);
				Renderer2D::drawLight(transf.getTransform(), plight, camera, (int)entity);
			}

			auto view5 = m_registry.view<DirLightComponent>();
			auto entity = view5.front();
			for (auto entity : view5) {
				auto dlight = view5.get<DirLightComponent>(entity);
				Renderer2D::drawDirLight(dlight);
			}
			Renderer2D::EndScene();
			
			
			depthPassAtmosphere(camera);
			MeshRenderer::GeometryPassGBuffer(camera);
			m_gbuffer_framebuffer->unbind();

			// SSAO PASS
			m_ssao_framebuffer->bind();
			RenderCommand::ClearColorBuffer();
			MeshRenderer::SSAOPass(camera, m_vp_width, m_vp_height, m_gbuffer_framebuffer->getColorAtt(0), m_gbuffer_framebuffer->getColorAtt(1), m_gbuffer_framebuffer->getColorAtt(4));
			m_ssao_framebuffer->unbind();

			m_ssao_blur_framebuffer->bind();
			RenderCommand::ClearColorBuffer();
			MeshRenderer::SSAOBlurPass(m_ssao_framebuffer->getColorAtt(0));
			m_ssao_blur_framebuffer->unbind();

			// LIGHTING PASS
			m_deferred_hdr_framebuffer->bind();
			RenderCommand::Clear();
			MeshRenderer::LighintgPass(camera, m_gbuffer_framebuffer->getColorAtt(0), m_gbuffer_framebuffer->getColorAtt(1), m_gbuffer_framebuffer->getColorAtt(2), m_gbuffer_framebuffer->getColorAtt(4), m_shadow_map->getDepthAtt(), m_ssao_blur_framebuffer->getColorAtt(0), cascade1, use_ssao);
			m_deferred_hdr_framebuffer->unbind();

			m_deferred_atmosphere_framebuffer->bind();
			renderAtmosphere(camera);
			m_deferred_atmosphere_framebuffer->unbind();

			applyToneMapping(camera.getExposure(), m_deferred_atmosphere_framebuffer->getColorAtt(0));
		}
		
	}

	void Scene::renderAtmosphere(EditorCamera& camera) {
		glm::vec3 enginePos = camera.getPosition();

		glm::vec3 atmospherePos = glm::vec3(enginePos.x, -enginePos.z, enginePos.y);

		glm::mat4 engineToAtmosphere = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f, // Atmosphere X = Engine X
			0.0f, 0.0f, 1.0f, 0.0f, // Atmosphere Z = Engine Y
			0.0f, -1.0f, 0.0f, 0.0f, // Atmosphere Y = Engine -Z
			0.0f, 0.0f, 0.0f, 1.0f
		);

		glm::mat4 atmosphere_model_from_view = engineToAtmosphere * glm::inverse(camera.getViewMatrix());
		glm::mat4 atmosphere_view_from_model = engineToAtmosphere * camera.getViewMatrix();
		glm::mat4 view_from_clip_camera = glm::inverse(camera.getProjection());
		glm::mat4 clip_from_view_camera = camera.getProjection();
		glm::mat4 inverse_view = glm::inverse(camera.getViewMatrix());

		/// this is engine space
		glm::vec3 to_sun_dir_y_up = sun_direction;
		glm::vec3 to_sun_dir_z_up = glm::normalize(glm::vec3(sun_direction.x, -sun_direction.z, sun_direction.y));
		glm::vec3 earth_center = glm::vec3(0.0f, 0.0f, -6360.0f);
		glm::vec3 white_point = glm::vec3(1.08236f, 0.96829f, 0.94934f);
		glm::vec2 sun_size = glm::vec2(tan(m_sun_angular_radius), cos(m_sun_angular_radius));


		uint32_t transmittance = m_atmosphere->getTransmittanceTex()->getRendererID();
		uint32_t scattering = m_atmosphere->getScatteringTex()->getRendererID();
		uint32_t mie_scattering = m_atmosphere->getMieScatteringTex()->getRendererID();
		uint32_t irradiance = m_atmosphere->getIrradianceTex()->getRendererID();

		Renderer2D::drawAtmosphere(
			m_deferred_atmosphere_framebuffer->getSpecification().width,
			m_deferred_atmosphere_framebuffer->getSpecification().height,
			atmosphere_model_from_view,
			view_from_clip_camera,
			atmosphere_view_from_model,
			clip_from_view_camera,
			inverse_view,
			atmospherePos,
			white_point,
			earth_center,
			to_sun_dir_z_up,
			to_sun_dir_y_up,
			sun_size,
			camera.getExposure(),
			transmittance,
			scattering,
			mie_scattering,
			irradiance,
			m_deferred_hdr_framebuffer->getColorAtt(),
			m_gbuffer_framebuffer->getDepthAtt(),
			m_gbuffer_framebuffer->getColorAtt(0),
			m_gbuffer_framebuffer->getColorAtt(1),
			m_ssao_blur_framebuffer->getColorAtt(),
			m_gbuffer_framebuffer->getColorAtt(4),
			m_shadow_map->getDepthAtt()
		);
	}

	void Scene::depthPassAtmosphere(EditorCamera& camera) {
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		glDrawBuffers(0, nullptr);
		
		glm::vec3 enginePos = camera.getPosition();

		glm::vec3 atmospherePos = glm::vec3(enginePos.x, -enginePos.z, enginePos.y);

		glm::mat4 engineToAtmosphere = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f, 
			0.0f, 0.0f, 1.0f, 0.0f, 
			0.0f, -1.0f, 0.0f, 0.0f, 
			0.0f, 0.0f, 0.0f, 1.0f
		);

		glm::mat4 atmosphere_model_from_view = engineToAtmosphere * glm::inverse(camera.getViewMatrix());
		glm::mat4 atmosphere_view_from_model = glm::inverse(atmosphere_model_from_view);
		glm::mat4 view_from_clip_camera = glm::inverse(camera.getProjection());
		glm::mat4 clip_from_view_camera = camera.getProjection();


		glm::vec3 to_sun_dir_z_up = glm::normalize(glm::vec3(sun_direction.x, -sun_direction.z, sun_direction.y));
		glm::vec3 earth_center = glm::vec3(0.0f, 0.0f, -6360.0f);
		glm::vec3 white_point = glm::vec3(1.08236f, 0.96829f, 0.94934f);
		glm::vec2 sun_size = glm::vec2(tan(m_sun_angular_radius), cos(m_sun_angular_radius));


		uint32_t transmittance = m_atmosphere->getTransmittanceTex()->getRendererID();
		uint32_t scattering = m_atmosphere->getScatteringTex()->getRendererID();
		uint32_t mie_scattering = m_atmosphere->getMieScatteringTex()->getRendererID();
		uint32_t irradiance = m_atmosphere->getIrradianceTex()->getRendererID();

		Renderer2D::depthPassAtmosphere(
			m_deferred_atmosphere_framebuffer->getSpecification().width,
			m_deferred_atmosphere_framebuffer->getSpecification().height,
			atmosphere_model_from_view,
			view_from_clip_camera,
			atmosphere_view_from_model,
			clip_from_view_camera,
			atmospherePos,
			earth_center
		);

		GLenum bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, bufs);
	}

	void Scene::onSunMovedUpdate() {
		/*const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
		glm::vec2 delta = (mouse - m_initial_mouse_pos) * 0.003f;*/
	}

	void Scene::applyToneMapping(float exposure, uint32_t hdr_texture) {
		m_deferred_final_ldr_framebuffer->bind();

		Renderer2D::applyToneMapping(hdr_texture, exposure);

		m_deferred_final_ldr_framebuffer->unbind();
	}

	void Scene::onViewportResize(uint32_t width, uint32_t height) {
		m_vp_width = width;
		m_vp_height = height;

		auto view = m_registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& camera = view.get<CameraComponent>(entity);
			if (!camera.fixed_aspect_ratio) {
				camera.camera.setViewportSize(width, height);
			}
		}
	}

	void Scene::resizeFramebuffers(uint32_t width, uint32_t height) {
		m_msaa_framebuffer->resize(width, height);
		m_gbuffer_framebuffer->resize(width, height);
		m_ssao_framebuffer->resize(width, height);
		m_ssao_blur_framebuffer->resize(width, height);
		m_deferred_final_ldr_framebuffer->resize(width, height);
		m_deferred_hdr_framebuffer->resize(width, height);
		m_atm_fbo->resize(width, height);
	}

	void Scene::render2DPassEdit(EditorCamera& camera) {
		Renderer2D::BeginScene(camera, m_plights, m_dlight);
		auto view = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : view) {
			auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::drawSprite(transform.getTransform(), sprite, (int)entity);
		}

		auto view4 = m_registry.view<TransformComponent, PointLightComponent>();
		for (auto entity : view4) {
			auto [transf, plight] = view4.get<TransformComponent, PointLightComponent>(entity);
			Renderer2D::drawLight(transf.getTransform(), plight, camera, (int)entity);
		}

		auto view5 = m_registry.view<DirLightComponent>();
		auto entity = view5.front();
		for (auto entity : view5) {
			//IARA_CORE_INFO("Directional Light detected!!");
			auto dlight = view5.get<DirLightComponent>(entity);
			Renderer2D::drawDirLight(dlight);
		}
		Renderer2D::EndScene();
	}

	void Scene::renderShadowmapCascadesPass() {
		/// Cascade 1
		glm::mat4 light_vp = m_cascades[0];
		m_shadow_map->bind();
		RenderCommand::Clear();
		//glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		
		MeshRenderer::ShadowMapPass(light_vp);

		glCullFace(GL_BACK);
		m_shadow_map->unbind();

		/// Cascade 2
		light_vp = m_cascades[1];
		m_shadow_map_cascade2->bind();
		RenderCommand::Clear();
		//glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		MeshRenderer::ShadowMapPass(light_vp);

		glCullFace(GL_BACK);
		m_shadow_map_cascade2->unbind();

		/// Cascade 3
		light_vp = m_cascades[2];
		m_shadow_map_cascade3->bind();
		RenderCommand::Clear();
		//glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		MeshRenderer::ShadowMapPass(light_vp);

		glCullFace(GL_BACK);
		m_shadow_map_cascade3->unbind();
		
		/// Cascade 4
		light_vp = m_cascades[3];
		m_shadow_map_cascade4->bind();
		RenderCommand::Clear();
		//glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		MeshRenderer::ShadowMapPass(light_vp);

		glCullFace(GL_BACK);
		m_shadow_map_cascade4->unbind();
		
		
	}

	void Scene::renderToShadowMapPass(const glm::mat4& light_vp) {
		m_shadow_map->bind();
		RenderCommand::Clear();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		MeshRenderer::ShadowMapPass(light_vp);

		glCullFace(GL_BACK);
		m_shadow_map->unbind();
	}

	void Scene::render3DPassEdit(EditorCamera& camera, const glm::mat4& light_vp) {		
		uint32_t shadowmap = m_shadow_map->getDepthAtt();
		MeshRenderer::ForwardPass(camera, light_vp, shadowmap);
	}

	void Scene::render2DPassRuntime(Camera& camera, const glm::mat4& camera_transform)
	{
		Renderer2D::BeginScene(camera, camera_transform, m_plights, m_dlight);

		auto view1 = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : view1) {
			auto [transform, sprite] = view1.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::drawSprite(transform.getTransform(), sprite, (int)entity);
		}

		auto view4 = m_registry.view<TransformComponent, PointLightComponent>();
		for (auto entity : view4) {
			auto [transf, plight] = view4.get<TransformComponent, PointLightComponent>(entity);
			Renderer2D::drawLight(transf.getTransform(), plight, camera, (int)entity);
		}

		auto view5 = m_registry.view<DirLightComponent>();
		auto entity = view5.front();
		for (auto entity : view5) {
			auto dlight = view5.get<DirLightComponent>(entity);
			Renderer2D::drawDirLight(dlight);
		}
		Renderer2D::EndScene();
	}

	void Scene::render3DPassRuntime(Camera& camera, const glm::mat4& camera_transform, const glm::mat4& light_vp) {
		uint32_t shadowmap = m_shadow_map->getDepthAtt();
		MeshRenderer::ForwardPass(camera, camera_transform, light_vp, shadowmap);
	}

	void Scene::renderShadowMapToColorFBO() {
		m_shadowmap_quad->bind();
		RenderCommand::Clear();
		
		uint32_t shadowmap = m_shadow_map_cascade2->getDepthAtt();
		Renderer2D::drawShadowMapToQuad(shadowmap);

		m_shadowmap_quad->unbind();
	}

	Entity Scene::getPrimaryCameraEntity() {
		auto view = m_registry.view<CameraComponent>();
		for (auto entity : view) {
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.primary) {
				return Entity{ entity, this };
			}
		}
		return {};
	}

	void Scene::initializeShadowMap() {
		int size = 4096;

		std::string name = "ShadowMap ";
		FramebufferSpecification specs;
		specs.attachments = { FramebufferTextureFormat::DEPTH_COMPONENT };
		specs.width = size;
		specs.height = size;
		specs.samples = 1;
		m_shadow_map = Framebuffer::Create(specs, name);

		name = "ShadowMap -C2 ";
		m_shadow_map_cascade2 = Framebuffer::Create(specs, name);

		name = "ShadowMap -C3 ";
		m_shadow_map_cascade3 = Framebuffer::Create(specs, name);

		name = "ShadowMap -C4 ";
		m_shadow_map_cascade4 = Framebuffer::Create(specs, name);

		std::string name2 = "ShadowMapQuad ";
		FramebufferSpecification specs2;
		specs2.attachments = { FramebufferTextureFormat::RGBA16F };
		specs2.width = size;
		specs2.height = size;
		specs2.samples = 1;
		m_shadowmap_quad = Framebuffer::Create(specs2, name2);


		float camera_far = 1000.0f;
		m_shadow_cascade_levels.push_back(camera_far / 50.0f);
		m_shadow_cascade_levels.push_back(camera_far / 15.0f);
		m_shadow_cascade_levels.push_back(camera_far / 5.0f);
		m_cascades.resize(4);
	}

	void Scene::computeLightSpaceMatrices(const EditorCamera& camera) {
		for (int i = 0; i < m_shadow_cascade_levels.size() + 1; i++) {
			if (i == 0) {
				m_cascades[i] = computeCascadeMatrix(0.1f, m_shadow_cascade_levels[i], camera);
			}
			else if (i < m_shadow_cascade_levels.size()) {
				m_cascades[i] = computeCascadeMatrix(m_shadow_cascade_levels[i - 1], m_shadow_cascade_levels[i], camera);
			}
			else {
				m_cascades[i] = computeCascadeMatrix(m_shadow_cascade_levels[i - 1], 1000.0f, camera);
			}
		}
	}

	glm::mat4 Scene::computeCascadeMatrix(float near_clip, float far_clip, const EditorCamera& camera) {
		glm::vec3 to_sun = glm::normalize(glm::vec3(sun_direction.x, sun_direction.y, sun_direction.z));
		glm::mat4 proj = glm::perspective(glm::radians(80.0f), (float)(m_vp_width / m_vp_height), near_clip, far_clip);
		std::vector<glm::vec4> frostum_corners = computeFrostumCornersWS(proj * camera.getViewMatrix());
		glm::vec3 target = glm::vec3(0.0f);
		for (const auto& c : frostum_corners) {
			target += glm::vec3(c);
		}
		target /= frostum_corners.size();
		//IARA_CORE_INFO("Target vector {0}, {1}, {2}", target.x, target.y, target.z);
		glm::vec3 light_position = target + to_sun * shadow_map_light_distance;
		glm::vec3 up_vector = glm::vec3(0.0f, 1.0f, 0.0f);
		/*if (std::abs(glm::dot(to_sun, up_vector)) > 0.99f) {
			up_vector = glm::vec3(0.0f, 0.0f, 1.0f);
		}*/
		glm::mat4 lightView = glm::lookAt(light_position, camera.getPosition(), up_vector);


		float min_x = std::numeric_limits<float>::max();
		float max_x = std::numeric_limits<float>::lowest();
		float min_y = std::numeric_limits<float>::max();
		float max_y = std::numeric_limits<float>::lowest();
		float min_z = std::numeric_limits<float>::max();
		float max_z = std::numeric_limits<float>::lowest();
		for (const auto& c : frostum_corners) {
			glm::vec4 trf = lightView * c;
			min_x = std::min(min_x, trf.x);
			max_x = std::max(max_x, trf.x);

			min_y = std::min(min_y, trf.y);
			max_y = std::max(max_y, trf.y);

			min_z = std::min(min_z, trf.z);
			max_z = std::max(max_z, trf.z);
		}

		float z_padding = 100.0f;
		/*if (min_z < 0) {
			min_z *= z_mult;
		}
		else {
			min_z /= z_mult;
		}
		if (max_z < 0) {
			max_z /= z_mult;
		}
		else {
			max_z *= z_mult;
		}*/

		float r = 8.0f;
		

		/// nu sunt buni parametrii min_x, min_y etc. cu date hardcodate macar merge
		//glm::mat4 lightProjection = glm::ortho(-r, r, -r, r, min_z - z_padding, max_z + z_padding);
		glm::mat4 lightProjection = glm::ortho(min_x, max_x, min_y, max_y, 0.1f, 100.0f);

		return lightProjection * lightView;
	}

	std::vector<glm::vec4> Scene::computeFrostumCornersWS(const glm::mat4& view_proj) {
		std::vector<glm::vec4> corners;
		glm::mat4 view_proj_inverse = glm::inverse(view_proj);
		
		for (int x = 0; x < 2; x++) {
			for (int y = 0; y < 2; y++) {
				for (int z = 0; z < 2; z++) {
					glm::vec4 pt = view_proj_inverse * glm::vec4(x * 2.0f - 1.0f, y * 2.0f - 1.0f, z * 2.0f - 1.0f, 1.0f);
					corners.push_back(pt / pt.w);
				}
			}
		}

		return corners;
	}

	void Scene::initializeAtmosphere() {
		constexpr double kPi = 3.1415926;
		constexpr double kSunAngularRadius = 0.00935 / 2.0;
		constexpr double kSunSolidAngle = kPi * kSunAngularRadius * kSunAngularRadius;
		constexpr double kLengthUnitInMeters = 1000.0;

		// Values from "Reference Solar Spectral Irradiance: ASTM G-173", ETR column
		// (see http://rredc.nrel.gov/solar/spectra/am1.5/ASTMG173/ASTMG173.html),
		// summed and averaged in each bin (e.g. the value for 360nm is the average
		// of the ASTM G-173 values for all wavelengths between 360 and 370nm).
		// Values in W.m^-2.
		constexpr int kLambdaMin = 360;
		constexpr int kLambdaMax = 830;
		constexpr double kSolarIrradiance[48] = {
		  1.11776, 1.14259, 1.01249, 1.14716, 1.72765, 1.73054, 1.6887, 1.61253,
		  1.91198, 2.03474, 2.02042, 2.02212, 1.93377, 1.95809, 1.91686, 1.8298,
		  1.8685, 1.8931, 1.85149, 1.8504, 1.8341, 1.8345, 1.8147, 1.78158, 1.7533,
		  1.6965, 1.68194, 1.64654, 1.6048, 1.52143, 1.55622, 1.5113, 1.474, 1.4482,
		  1.41018, 1.36775, 1.34188, 1.31429, 1.28303, 1.26758, 1.2367, 1.2082,
		  1.18737, 1.14683, 1.12362, 1.1058, 1.07124, 1.04992
		};
		// Values from http://www.iup.uni-bremen.de/gruppen/molspec/databases/
		// referencespectra/o3spectra2011/index.html for 233K, summed and averaged in
		// each bin (e.g. the value for 360nm is the average of the original values
		// for all wavelengths between 360 and 370nm). Values in m^2.
		constexpr double kOzoneCrossSection[48] = {
		  1.18e-27, 2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27, 5.52e-27,
		  8.451e-27, 1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26, 7.752e-26, 9.016e-26,
		  1.48e-25, 1.602e-25, 2.139e-25, 2.755e-25, 3.091e-25, 3.5e-25, 4.266e-25,
		  4.672e-25, 4.398e-25, 4.701e-25, 5.019e-25, 4.305e-25, 3.74e-25, 3.215e-25,
		  2.662e-25, 2.238e-25, 1.852e-25, 1.473e-25, 1.209e-25, 9.423e-26, 7.455e-26,
		  6.566e-26, 5.105e-26, 4.15e-26, 4.228e-26, 3.237e-26, 2.451e-26, 2.801e-26,
		  2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27
		};
		// From https://en.wikipedia.org/wiki/Dobson_unit, in molecules.m^-2.
		constexpr double kDobsonUnit = 2.687e20;
		// Maximum number density of ozone molecules, in m^-3 (computed so at to get
		// 300 Dobson units of ozone - for this we divide 300 DU by the integral of
		// the ozone density profile defined below, which is equal to 15km).
		constexpr double kMaxOzoneNumberDensity = 300.0 * kDobsonUnit / 15000.0;
		// Wavelength independent solar irradiance "spectrum" (not physically
		// realistic, but was used in the original implementation).
		constexpr double kConstantSolarIrradiance = 1.5;
		constexpr double kBottomRadius = 6360000.0;
		constexpr double kTopRadius = 6400000.0;
		constexpr double kRayleigh = 1.24062e-6;
		constexpr double kRayleighScaleHeight = 8000.0;
		constexpr double kMieScaleHeight = 1200.0;
		constexpr double kMieAngstromAlpha = 0.0;
		constexpr double kMieAngstromBeta = 5.328e-3;
		constexpr double kMieSingleScatteringAlbedo = 0.9;
		constexpr double kMiePhaseFunctionG = 0.8;
		constexpr double kGroundAlbedo = 0.1;
		const double max_sun_zenith_angle = 102.0 / 180.0 * kPi;

		DensityProfileLayer
			rayleigh_layer(0.0, 1.0, -1.0 / kRayleighScaleHeight, 0.0, 0.0);
		DensityProfileLayer mie_layer(0.0, 1.0, -1.0 / kMieScaleHeight, 0.0, 0.0);
		// Density profile increasing linearly from 0 to 1 between 10 and 25km, and
		// decreasing linearly from 1 to 0 between 25 and 40km. This is an approximate
		// profile from http://www.kln.ac.lk/science/Chemistry/Teaching_Resources/
		// Documents/Introduction%20to%20atmospheric%20chemistry.pdf (page 10).
		std::vector<DensityProfileLayer> ozone_density;
		ozone_density.push_back(
			DensityProfileLayer(25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0));
		ozone_density.push_back(
			DensityProfileLayer(0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0));

		atmosphere::DensityProfileLayer rayleigh_layer_atm(0.0, 1.0, -1.0 / kRayleighScaleHeight, 0.0, 0.0);
		atmosphere::DensityProfileLayer mie_layer_atm(0.0, 1.0, -1.0 / kMieScaleHeight, 0.0, 0.0);
		std::vector<atmosphere::DensityProfileLayer> ozone_density_atm;
		ozone_density_atm.push_back(
			atmosphere::DensityProfileLayer(25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0));
		ozone_density_atm.push_back(
			atmosphere::DensityProfileLayer(0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0));

		std::vector<double> wavelengths;
		std::vector<double> solar_irradiance;
		std::vector<double> rayleigh_scattering;
		std::vector<double> mie_scattering;
		std::vector<double> mie_extinction;
		std::vector<double> absorption_extinction;
		std::vector<double> ground_albedo;
		for (int l = kLambdaMin; l <= kLambdaMax; l += 10) {
			double lambda = static_cast<double>(l) * 1e-3;  // micro-meters
			double mie = kMieAngstromBeta / kMieScaleHeight * pow(lambda, -kMieAngstromAlpha);
			wavelengths.push_back(l);
			//solar_irradiance.push_back(kSolarIrradiance[(l - kLambdaMin) / 10]);
			solar_irradiance.push_back(kConstantSolarIrradiance);

			rayleigh_scattering.push_back(kRayleigh * pow(lambda, -4));
			mie_scattering.push_back(mie * kMieSingleScatteringAlbedo);
			mie_extinction.push_back(mie);
			//absorption_extinction.push_back(kMaxOzoneNumberDensity * kOzoneCrossSection[(l - kLambdaMin) / 10]);
			absorption_extinction.push_back(0);
			ground_albedo.push_back(kGroundAlbedo);
		}

		m_atmosphere.reset(new Atmosphere(wavelengths, solar_irradiance, kSunAngularRadius,
			kBottomRadius, kTopRadius, { rayleigh_layer }, rayleigh_scattering,
			{ mie_layer }, mie_scattering, mie_extinction, kMiePhaseFunctionG,
			ozone_density, absorption_extinction, ground_albedo, max_sun_zenith_angle,
			kLengthUnitInMeters, 3));
		m_atmosphere->init(3);
	}

	void Scene::initializeFramebuffers() {

		FramebufferSpecification fb_spec;
		fb_spec.attachments = { 
			FramebufferTextureFormat::RGBA8,
			FramebufferTextureFormat::RED_INTEGER,
			FramebufferTextureFormat::DEPTH24STENCIL8 };
		fb_spec.width = 1920;
		fb_spec.height = 1080;
		m_msaa_framebuffer = Framebuffer::CreateMSAA(fb_spec);


		/// Position, Normal, Diffuse/Specular, EntityID, Metalness & Roughness
		fb_spec.attachments = { 
			FramebufferTextureFormat::RGBA16F,
			FramebufferTextureFormat::RGBA16F,
			FramebufferTextureFormat::RGBA16F,
			FramebufferTextureFormat::RGBA16F,
			FramebufferTextureFormat::RED_INTEGER,
			FramebufferTextureFormat::DEPTH24STENCIL8 };
		fb_spec.width = 1920;
		fb_spec.height = 1080;
		m_gbuffer_framebuffer = Framebuffer::Create(fb_spec, "G Buffer ");

		fb_spec.attachments = { FramebufferTextureFormat::RGBA16F };
		m_deferred_hdr_framebuffer = Framebuffer::Create(fb_spec, "Deferred HDR Lighitng Calculation ");

		m_deferred_atmosphere_framebuffer = Framebuffer::Create(fb_spec, "Deffered HDR Atmosphere + Geometry Shading ");

		fb_spec.attachments = { FramebufferTextureFormat::RGBA8 };
		m_deferred_final_ldr_framebuffer = Framebuffer::Create(fb_spec, "LDR Post Lighting Calculation");
		

		fb_spec.attachments = { FramebufferTextureFormat::RED };
		m_ssao_framebuffer = Framebuffer::Create(fb_spec, "SSAO ");

		fb_spec.attachments = { FramebufferTextureFormat::RED };
		m_ssao_blur_framebuffer = Framebuffer::Create(fb_spec, "SSAO_BLUR ");

		fb_spec.attachments = { FramebufferTextureFormat::RGBA16F };
		fb_spec.width = 848;
		fb_spec.height = 480;
		m_atm_fbo = Framebuffer::Create(fb_spec, " Atmosphere ");
	}

	const uint32_t Scene::getFinalRenderedTexture() {
		//return m_deferred_final_ldr_framebuffer->getColorAtt(0);
		if (rendering_type == RenderingType::MSAA) {
			return m_msaa_framebuffer->getColorAtt(0);
		}
		else if (rendering_type == RenderingType::DEFERRED) {
			return m_deferred_final_ldr_framebuffer->getColorAtt(0);
		}

	}

	template<typename T>
	void Scene::onComponentAdded(Entity entity, T& component) {
		static_assert(false);
	}

	template<>
	void Scene::onComponentAdded<TagComponent>(Entity entity, TagComponent& component) {

	}

	template<>
	void Scene::onComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) {

	}

	template<>
	void Scene::onComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component) {

	}

	template<>
	void Scene::onComponentAdded<CameraComponent>(Entity entity, CameraComponent& component) {
		component.camera.setViewportSize(m_vp_width, m_vp_height);
	}

	template<>
	void Scene::onComponentAdded<Texture2DComponent>(Entity entity, Texture2DComponent& component) {

	}

	template<>
	void Scene::onComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) {

	}

	template<>
	void Scene::onComponentAdded<cube3DComponent>(Entity entity, cube3DComponent& component) {

	}

	template<>
	void Scene::onComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component) {

	}

	template<>
	void Scene::onComponentAdded<DirLightComponent>(Entity entity, DirLightComponent& component) {

	}

	template<>
	void Scene::onComponentAdded<MeshComponent>(Entity entity, MeshComponent& component) {

	}
}