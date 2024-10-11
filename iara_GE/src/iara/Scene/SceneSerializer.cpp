#include "ir_pch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Component.h"

#include <yaml-cpp\yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3) return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4> {
		static Node encode(const glm::vec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4) return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}


namespace iara {

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : m_scene{ scene } {

	}

	static void serializeEntity(YAML::Emitter& out, Entity entity) {
		out << YAML::BeginMap; /// Entity
		out << YAML::Key << "Entity" << YAML::Value << "12345"; /// TODO entity uuid

		if (entity.hasComponent<TagComponent>()) {
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			auto& tag = entity.getComponent<TagComponent>().tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap;
		}

		if (entity.hasComponent<TransformComponent>()) {
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			auto& tc = entity.getComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.scale;

			out << YAML::EndMap;
		}

		if (entity.hasComponent<CameraComponent>()) {
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; /// cc

			auto& cc = entity.getComponent<CameraComponent>();
			
			auto& camera = cc.camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; /// camera props
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.getProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.getPerspectiveVerticalFov();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.getPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.getPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.getOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.getOrthoNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.getOrthoFarClip();
			out << YAML::EndMap; /// camera props

			out << YAML::Key << "Primary" << YAML::Value << cc.primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.fixed_aspect_ratio;

			out << YAML::EndMap; /// cc
		}

		if (entity.hasComponent<SpriteRendererComponent>()) {
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;

			auto& color = entity.getComponent<SpriteRendererComponent>().color;
			out << YAML::Key << "Color" << YAML::Value << color;

			out << YAML::EndMap;
		}

		/*if (entity.hasComponent<Texture2DComponent>()) {
			out << YAML::Key << "Texture2DComponent";
			out << YAML::BeginMap;

			auto& filepath = entity.getComponent<Texture2DComponent>().filepath;
			out << YAML::Key << "filepath" << YAML::Value << filepath;

			out << YAML::EndMap;
		}*/


		out << YAML::EndMap; /// Entity
	}

	void SceneSerializer::serialize(const std::string& path) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		auto view = m_scene->m_registry.view<entt::entity>();
		for (auto entityID : view) {
			Entity entity = { entityID, m_scene.get() };
			if (entity)
				serializeEntity(out, entity);
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();
	}

	void SceneSerializer::serializeRunTime(const std::string& path) {
		IARA_CORE_ASSERT(false)
	}

	bool SceneSerializer::deserialize(const std::string& path) {
		std::ifstream stream(path);
		std::stringstream strstream;
		strstream << stream.rdbuf();

		YAML::Node data = YAML::Load(strstream.str());
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		IARA_CORE_TRACE("Deserializing Scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities) {
			for (auto entity : entities) {
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComp = entity["TagComponent"];
				if (tagComp) {
					name = tagComp["Tag"].as<std::string>();
				}

				IARA_CORE_TRACE("Deserialized Entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_scene->createEntity(name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent) {
					/// all entities have a transform upon creation
					auto& tc = deserializedEntity.getComponent<TransformComponent>();
					tc.translation = transformComponent["Translation"].as<glm::vec3>();
					tc.rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComp = entity["CameraComponent"];
				if (cameraComp) {
					auto& cc = deserializedEntity.addComponent<CameraComponent>();

					auto& cameraProps = cameraComp["Camera"];

					cc.camera.setProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

					cc.camera.setPerspectiveVerticalFov(cameraProps["PerspectiveFOV"].as<float>());
					cc.camera.setPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.camera.setPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.camera.setOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.camera.setOrthoNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.camera.setOrthoFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.primary = cameraComp["Primary"].as<bool>();
					cc.fixed_aspect_ratio = cameraComp["FixedAspectRatio"].as<bool>();
				}

				auto spriteComp = entity["SpriteRendererComponent"];
				if (spriteComp) {
					auto& src = deserializedEntity.addComponent<SpriteRendererComponent>();
					src.color = spriteComp["Color"].as<glm::vec4>();
				}

				/*auto texComp = entity["Texture2DComponent"];
				if (texComp) {
					auto& tx2d = deserializedEntity.addComponent<Texture2DComponent>(texComp["filepath"].as<std::string>());
				}*/
			}
		}
		return true;
	}

	bool SceneSerializer::deserializeRunTime(const std::string& path) {
		IARA_CORE_ASSERT(false);
		return false;
	}

}