#pragma once

#include "iara/Renderer/Camera.h"
#include "ScriptableEntity.h"
#include "SceneCamera.h"
#include "iara\Renderer\Mesh.h"
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

		Ref<Texture2D> texture;
		std::string tex_path;
		glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
		float tiling_factor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color, const std::string& path)
			: color{ color }, tex_path{ path } {
			texture = Texture2D::Create(path);
		}
		SpriteRendererComponent(const glm::vec4& color)
			: color{ color } {
		}
		SpriteRendererComponent(const std::string& path)
			: tex_path{ path } {
			color = glm::vec4(1.0f);
			texture = Texture2D::Create(path);
		}

		operator glm::vec4& () { return color; }
		operator const glm::vec4& () const { return color; }
	};

	struct PointLight {
		glm::vec4 position = glm::vec4(0.0, 0.0, 0.0, 1.0);

		glm::vec4 ambient = glm::vec4(0.3, 0.3, 0.3, 1.0);
		glm::vec4 diffuse = glm::vec4(0.8, 0.3, 0.3, 1.0);
		glm::vec4 specular = glm::vec4(0.5, 0.5, 0.5, 1.0);

		float constant = 1.0f;
		float linear = 0.007f;
		float quadratic = 0.0002f;

		float padding;
	};

	struct DirLight {
		glm::vec4 direction = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);

		glm::vec4 ambient = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
		glm::vec4 diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	};

	struct cube3DComponent {
		Ref<Texture2D> texture;
		std::string filepath;
		glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

		int material_index = -1;

		cube3DComponent() = default;
		cube3DComponent(const cube3DComponent&) = default;
		cube3DComponent(const glm::vec4& color)
			: color{ color } {
			
		}
		cube3DComponent(const std::string& path)
			: filepath{path} {
			texture = Texture2D::Create(path);
			
		}

		operator glm::vec4& () { return color; }
		operator const glm::vec4& () const { return color; }
	};

	struct MeshComponent {
		std::string path;
		std::vector<Material> materials;
		bool first_pass = false;
	};

	struct Texture2DComponent {

		Ref<Texture2D> texture;
		std::string filepath;

		Texture2DComponent() = default;
		Texture2DComponent(const Texture2DComponent&) = default;
		Texture2DComponent(const std::string& path)
		{
			filepath = path;
			texture = Texture2D::Create(path);
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

	struct PointLightComponent {
		Ref<Texture2D> light_tex;

		PointLight plight;

		PointLightComponent() {
			light_tex = Texture2D::Create("Assets/Textures/LightTex2.png");
		}

	};

	struct DirLightComponent {
		DirLight dlight;
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