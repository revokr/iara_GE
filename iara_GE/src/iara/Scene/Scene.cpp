#include "ir_pch.h"
#include "Scene.h"

#include "Component.h"
#include "Entity.h"
#include "iara/Renderer/Renderer2D.h"
#include <glm/ext/matrix_transform.hpp>


namespace iara {

	Scene::Scene()
	{
		m_registry = {};
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

	void Scene::destroyEntity(Entity entity) {
		m_registry.destroy(entity);
	}

	void Scene::onUpdateRuntime(Timestep ts) {
		/// Update Scripts
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
		}

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

			Renderer2D::BeginScene(*main_camera, camera_transform);

			/*auto view = m_registry.view<TransformComponent, SpriteRendererComponent>();
			for (auto entity : view) {
				auto [transform, sprite] = view.get< TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::drawQuadC(transform.getTransform(), sprite.color);
			}*/

			/*auto view1 = m_registry.view<TransformComponent, Texture2DComponent>();
			for (auto entity : view1) {
				auto [transform, spr] = view1.get< TransformComponent, Texture2DComponent>(entity);

				Renderer2D::drawQuadT(transform.getTransform(), spr.texture);
			}*/

			Renderer2D::EndScene();
		}
	}

	void Scene::onUpdateEditor(Timestep ts, EditorCamera& camera) {

		Renderer2D::BeginScene(camera);
		auto view = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : view) {
			auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::drawSprite(transform.getTransform(), sprite, (int)entity);
		}
		Renderer2D::EndScene();

		/*auto view1 = m_registry.view<TransformComponent, Texture2DComponent>();
		for (auto entity : view1) {
			auto [transform, spr] = view1.get< TransformComponent, Texture2DComponent>(entity);

			Renderer2D::drawQuadT(transform.getTransform(), spr.texture);
		}*/

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

}