#pragma once

#include "Scene.h"
#include <entt.hpp>

namespace iara {

	class Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& addComponent(Args&&... args) {
			T& component = m_scene->m_registry.emplace<T>(m_entity_handle, std::forward<Args>(args)...);
			m_scene->onComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		void deleteComponent() {
			m_scene->m_registry.remove<T>(m_entity_handle);
		}

		template<typename T>
		T& getComponent() {
			return m_scene->m_registry.get<T>(m_entity_handle);
		}

		template<typename T>
		bool hasComponent() {
			return m_scene->m_registry.try_get<T>(m_entity_handle);
		}

		operator bool() const { return m_entity_handle != entt::null; }
		operator entt::entity() const { return m_entity_handle; }
		operator uint32_t() const { return (uint32_t)m_entity_handle; }
		bool operator==(const Entity& other) { return m_entity_handle == other.m_entity_handle && m_scene == other.m_scene; }
		bool operator!=(const Entity& other) { return !(*this == other); }
	private:
		entt::entity m_entity_handle{entt::null};
		Scene* m_scene = nullptr;
	};

}