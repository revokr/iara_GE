#include "ir_pch.h"
#include "Scene.h"

#include "Component.h"
#include "Entity.h"
#include "../Core/Timer.h"
#include "iara/Renderer/Renderer2D.h"
#include <glm/ext/matrix_transform.hpp>
#include <glad\glad.h>

#include "iara\Math\Math.h"


namespace iara {

	Scene::Scene()
	{
		m_registry = {};

		createDirLight("Sky Light");
		initializeMainFramebuffer();
		initializeShadowMap();
	}

	Scene::~Scene()
	{
	}

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
					main_camera =	   &camera.camera;
					camera_transform = transform.getTransform();
					break;
				}
			}
		}

		if (main_camera) {
			Timer timer;
			renderToShadowMapPass(cascade1);
			render_shadowmap_timer = timer.elapsedMilliseconds();

			m_main_framebuffer->bind();
			iara::Renderer2D::ResetStats();
			iara::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.5f, 1.0f });
			iara::RenderCommand::Clear();
			m_main_framebuffer->clearAttachment(1, -1);
			if (m_skybox) {
				glm::mat4 view3 = glm::mat4(glm::mat3(glm::inverse(camera_transform)));
				Renderer3D::drawSkyBox(main_camera->getProjection() * view3, m_skybox);
			}

			render3DPassRuntime(*main_camera, camera_transform, cascade1);

			render2DPassRuntime(*main_camera, camera_transform);
			m_main_framebuffer->unbind();
		}
	}

	void Scene::onUpdateEditor(Timestep ts, EditorCamera& camera) {
		//Timer timer;
		renderToShadowMapPass(cascade1);
		renderShadowMapToColorFBO();
		//render_shadowmap_timer = timer.elapsedMilliseconds();

		m_main_framebuffer->bind();
		iara::Renderer2D::ResetStats();
		iara::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.5f, 1.0f });
		iara::RenderCommand::Clear();
		m_main_framebuffer->clearAttachment(1, -1);

		if (m_skybox) {
			glm::mat4 view3 = glm::mat4(glm::mat3(camera.getViewMatrix()));
			Renderer3D::drawSkyBox(camera.getProjection() * view3, m_skybox);
		}

		render2DPassEdit(camera);
		render3DPassEdit(camera, cascade1);

		m_main_framebuffer->unbind();
	}

	void Scene::onViewportResize(uint32_t width, uint32_t height) {
		m_vp_width  = width;
		m_vp_height = height;

		auto view = m_registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& camera = view.get<CameraComponent>(entity);
			if (!camera.fixed_aspect_ratio) {
				camera.camera.setViewportSize(width, height);
			}
		}
	}

	void Scene::render2DPassEdit(EditorCamera& camera)
	{
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

	void Scene::renderToShadowMapPass(const glm::mat4& light_vp)
	{
		m_shadow_map->bind();
		RenderCommand::Clear();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		MeshRenderer::BeginShadowMapPass(light_vp);

		auto view_mesh = m_registry.view<TransformComponent, MeshComponent>();
		for (auto entity : view_mesh) {
			auto [transf, mesh] = view_mesh.get<TransformComponent, MeshComponent>(entity);
			MeshRenderer::drawMesh(transf.getTransform(), mesh, (int)entity);
		}

		MeshRenderer::EndShadowMapPass();

		m_shadow_map->unbind();

		glDisable(GL_CULL_FACE);
	}

	void Scene::render3DPassEdit(EditorCamera& camera, const glm::mat4& light_vp)
	{
		MeshRenderer::BeginSceneMesh(camera, light_vp);

		auto view_mesh = m_registry.view<TransformComponent, MeshComponent>();
		for (auto entity : view_mesh) {
			auto [transf, mesh] = view_mesh.get<TransformComponent, MeshComponent>(entity);
			MeshRenderer::drawMesh(transf.getTransform(), mesh, (int)entity);
		}
		uint32_t shadowmap = m_shadow_map->getDepthAtt();
		MeshRenderer::EndSceneMesh(shadowmap);
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

	void Scene::render3DPassRuntime(Camera& camera, const glm::mat4& camera_transform, const glm::mat4& light_vp)
	{
		MeshRenderer::BeginSceneMesh(camera, camera_transform, light_vp);

		auto view_mesh = m_registry.view<TransformComponent, MeshComponent>();
		for (auto entity : view_mesh) {
			auto [transf, mesh] = view_mesh.get<TransformComponent, MeshComponent>(entity);
			MeshRenderer::drawMesh(transf.getTransform(), mesh, (int)entity);
		}

		uint32_t shadowmap = m_shadow_map->getDepthAtt();
		MeshRenderer::EndSceneMesh(shadowmap);
	}

	void Scene::renderShadowMapToColorFBO() {
		m_shadowmap_quad->bind();
		//RenderCommand::Clear();
		//RenderCommand::SetClearColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		uint32_t shadowmap = m_shadow_map->getDepthAtt();
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

	void Scene::initializeShadowMap()
	{
		std::string name = "ShadowMap ";
		FramebufferSpecification specs;
		specs.attachments = { FramebufferTextureFormat::DEPTH_COMPONENT };
		specs.width  = 2048;
		specs.height = 2048;
		m_shadow_map = Framebuffer::Create(specs, name);

		std::string name2 = "ShadowMapQuad ";
		FramebufferSpecification specs2;
		specs2.attachments = { FramebufferTextureFormat::RGBA8 };
		specs2.width = 2048;
		specs2.height = 2048;
		m_shadowmap_quad = Framebuffer::Create(specs2, name2);

		glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 1000.0f);
		glm::mat4 lightView = glm::lookAt(glm::vec3(-30.0f, 230.0f, -1.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		cascade1 = lightProjection * lightView;
	}

	void Scene::initializeMainFramebuffer()
	{
		FramebufferSpecification fb_spec;
		fb_spec.attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER , FramebufferTextureFormat::Depth };
		fb_spec.width = 1280;
		fb_spec.height = 720;
		m_main_framebuffer = Framebuffer::CreateMSAA(fb_spec);
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
	void Scene::onComponentAdded<cube3DComponent>(Entity entity, cube3DComponent	& component) {

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