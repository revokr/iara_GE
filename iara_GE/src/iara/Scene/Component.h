#pragma once

#include "iara/Renderer/Camera.h"
#include "iara/Renderer/Texture.h"
#include "ScriptableEntity.h"
#include "SceneCamera.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm\gtx\quaternion.hpp>

namespace iara {

	struct TagComponent {
		std::string tag;

		TagComponent() = default;
		TagComponent(const TagComponent& other) = default;
		TagComponent(const std::string& tag)
			: tag{ tag } {}
	};

	struct TransformComponent {

		glm::vec3 translation = { 0.1f, 0.1f, 0.1f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& transl)
			: translation{ transl } {}

		glm::mat4 getTransform() {
			glm::mat4 rotation_mat = glm::toMat4(glm::quat(rotation));/*glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3{1.0f, 0.0f, 0.0f}) *
				glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3{ 0.0f, 1.0f, 0.0f }) *
				glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3{ 0.0f, 0.0f, 1.0f });*/

			return glm::translate(glm::mat4(1.0f), translation) * rotation_mat * glm::scale(glm::mat4(1.0f), scale);
		}
	};

	struct SpriteRendererComponent {

		glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: color{ color } {}

		operator glm::vec4& () { return color; }
		operator const glm::vec4& () const { return color; }
	};

	struct Texture2DComponent {

		Ref<Texture2D> texture;
		std::string filepath;

		Texture2DComponent() = default;
		Texture2DComponent(const Texture2DComponent&) = default;
		Texture2DComponent(const std::string& path)
		{
			filepath = path;
			texture = Texture2D::CreateRef(path);
		}

		operator Ref<Texture2D>& () { return texture; }
		operator const Ref<Texture2D>& () const { return texture; }
	};

	struct CameraComponent {

		SceneCamera camera;
		bool primary = true;
		bool fixed_aspect_ratio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent& other) = default;
	};

	struct NativeScriptComponent {
		ScriptableEntity* instance = nullptr;

		ScriptableEntity* (*instantiateScript)();
		void (*destroyScript)(NativeScriptComponent*);

		template<typename T>
		void bind() {
			instantiateScript = []() {return static_cast<ScriptableEntity*>(new T()); };
			destroyScript = [](NativeScriptComponent* nsc) {delete nsc->instance; nsc->instance = nullptr; };
		}
	};

}