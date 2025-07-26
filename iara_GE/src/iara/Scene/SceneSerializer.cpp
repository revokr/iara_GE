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
			out << YAML::Key << "ProjectionType" << YAML::Value << (uint32_t)camera.getProjectionType();
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

			auto& path = entity.getComponent<SpriteRendererComponent>().tex_path;
			if (!path.empty()) out << YAML::Key << "Tex_Path" << YAML::Value << path;

			auto& tiling_factor = entity.getComponent<SpriteRendererComponent>().tiling_factor;
			out << YAML::Key << "TilingFactor" << YAML::Value << tiling_factor;

			out << YAML::EndMap;
		}

		if (entity.hasComponent<cube3DComponent>()) {
			out << YAML::Key << "cube3DComponent";
			out << YAML::BeginMap;

			auto& color = entity.getComponent<cube3DComponent>().color;
			out << YAML::Key << "Color" << YAML::Value << color;

			auto& path = entity.getComponent<cube3DComponent>().filepath;
			if (!path.empty()) out << YAML::Key << "Tex_Path" << YAML::Value << path;

			out << YAML::EndMap;
		}

		if (entity.hasComponent<PointLightComponent>()) {
			out << YAML::Key << "PointLightComponent";
			out << YAML::BeginMap;

			PointLight plight = entity.getComponent<PointLightComponent>().plight;
			auto& ambient = plight.ambient;
			auto& diffuse = plight.diffuse;
			auto& specular = plight.specular;
			out << YAML::Key << "ambient" << YAML::Value << ambient;
			out << YAML::Key << "diffuse" << YAML::Value << diffuse;
			out << YAML::Key << "specular" << YAML::Value << specular;

			out << YAML::Key << "constant" << YAML::Value << plight.constant;
			out << YAML::Key << "linear" << YAML::Value << plight.linear;
			out << YAML::Key << "quadratic" << YAML::Value << plight.quadratic;

			out << YAML::EndMap;
		}

		if (entity.hasComponent<DirLightComponent>()) {
			out << YAML::Key << "DirLightComponent";
			out << YAML::BeginMap;

			DirLight dlight = entity.getComponent<DirLightComponent>().dlight;
			auto& direction = dlight.direction;
			auto& ambient = dlight.ambient;
			auto& diffuse = dlight.diffuse;
			auto& specular = dlight.specular;
			out << YAML::Key << "direction" << YAML::Value << direction;
			out << YAML::Key << "ambient" << YAML::Value << ambient;
			out << YAML::Key << "diffuse" << YAML::Value << diffuse;
			out << YAML::Key << "specular" << YAML::Value << specular;

			out << YAML::EndMap;
		}

		if (entity.hasComponent<MeshComponent>()) {
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap;

			std::string path = entity.getComponent<MeshComponent>().path;
			out << YAML::Key << "path" << YAML::Value << path;
			std::vector<Material> materials = entity.getComponent<MeshComponent>().materials;
			if (materials.size() > 0) {
				out << YAML::Key << "Materials" << YAML::BeginMap;
				for (int i = 0; i < materials.size(); i++) {
					auto diffuse = materials[i].diffuse;
					auto diffuse_map_path = materials[i].diffuse_map->getPath();
					auto shininess = materials[i].shininess;
					auto specular_map_path = materials[i].specular_map->getPath();
					auto normal_map_path = materials[i].normal_map->getPath();

					out << YAML::Key << "Material" + std::to_string(i);
					out << YAML::BeginMap;

					out << YAML::Key << "diffuse" << YAML::Value << diffuse;
					out << YAML::Key << "diffuse_map_path" << YAML::Value << diffuse_map_path;
					out << YAML::Key << "shininess" << YAML::Value << shininess;
					out << YAML::Key << "specular_map_path" << YAML::Value << specular_map_path;
					out << YAML::Key << "normal_map_path" << YAML::Value << normal_map_path;

					out << YAML::EndMap;

				}
				out << YAML::EndMap;
			}

			out << YAML::EndMap;
			
		}

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

		auto view2 = m_scene->m_registry.view<PointLightComponent>();
		for (auto ent : view2) {
			m_scene->increasePointLights();
		}

		out << YAML::EndSeq;

		out << YAML::Key << "CubemapPath" << YAML::Value << m_scene->m_skybox_path;

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


					cc.camera.setPerspectiveVerticalFov(cameraProps["PerspectiveFOV"].as<float>());
					cc.camera.setPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.camera.setPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.camera.setOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.camera.setOrthoNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.camera.setOrthoFarClip(cameraProps["OrthographicFar"].as<float>());
					
					cc.camera.setProjectionType(cameraProps["ProjectionType"].as<uint32_t>());

					cc.primary = cameraComp["Primary"].as<bool>();
					cc.fixed_aspect_ratio = cameraComp["FixedAspectRatio"].as<bool>();

				}

				auto spriteComp = entity["SpriteRendererComponent"];
				if (spriteComp) {
					if (spriteComp["Tex_Path"]) {
						auto& src = deserializedEntity.addComponent<SpriteRendererComponent>(spriteComp["Tex_Path"].as<std::string>());
						if (spriteComp["TilingFactor"])
							src.tiling_factor = spriteComp["TilingFactor"].as<float>();
					}
					else if (spriteComp["Color"]) {
						auto& src = deserializedEntity.addComponent<SpriteRendererComponent>(spriteComp["Color"].as<glm::vec4>());
						if (spriteComp["TilingFactor"])
							src.tiling_factor = spriteComp["TilingFactor"].as<float>();
					}
					else {
						auto& src = deserializedEntity.addComponent<SpriteRendererComponent>();
						if (spriteComp["TilingFactor"])
							src.tiling_factor = spriteComp["TilingFactor"].as<float>();
					}
				}

				auto cubeComp = entity["cube3DComponent"];
				if (cubeComp) {
					if (cubeComp["Tex_Path"]) {
						auto& src = deserializedEntity.addComponent<cube3DComponent>(cubeComp["Tex_Path"].as<std::string>());
					}
					else if (cubeComp["Color"])
						deserializedEntity.addComponent<cube3DComponent>(cubeComp["Color"].as<glm::vec4>());
				}

				auto plightComp = entity["PointLightComponent"];
				if (plightComp) {
					m_scene->increasePointLights();
					auto& src = deserializedEntity.addComponent<PointLightComponent>();
					src.plight.ambient = plightComp["ambient"].as<glm::vec4>();
					src.plight.diffuse = plightComp["diffuse"].as<glm::vec4>();
					src.plight.specular = plightComp["specular"].as<glm::vec4>();
					src.plight.constant = plightComp["constant"].as<float>();
					src.plight.linear = plightComp["linear"].as<float>();
					src.plight.quadratic = plightComp["quadratic"].as<float>();
				}

				auto dlightComp = entity["DirLightComponent"];
				if (dlightComp) {
					auto& src = deserializedEntity.addComponent<DirLightComponent>();
					src.dlight.direction = dlightComp["direction"].as<glm::vec4>();
					src.dlight.ambient = dlightComp["ambient"].as<glm::vec4>();
					src.dlight.diffuse = dlightComp["diffuse"].as<glm::vec4>();
					src.dlight.specular = dlightComp["specular"].as<glm::vec4>();
				}

				auto meshComp = entity["MeshComponent"];
				if (meshComp) {
					auto& mshcmp = deserializedEntity.addComponent<MeshComponent>();
					mshcmp.path = meshComp["path"].as<std::string>();
					auto materials = meshComp["Materials"];
					if (materials) {
						Ref<Texture2D> white_tex = Texture2D::Create(1, 1);
						for (auto material : materials) {
							auto& materialValues = material.second;
							Material mat;
							mat.diffuse = materialValues["diffuse"].as<glm::vec4>();
							if (materialValues["diffuse_map_path"].as<std::string>() != "") {
								mat.diffuse_map = Texture2D::Create(materialValues["diffuse_map_path"].as<std::string>());
							}
							else {
								mat.diffuse_map = white_tex;
							}
							mat.shininess = materialValues["shininess"].as<float>();
							if (materialValues["specular_map_path"].as<std::string>() != "") {
								mat.specular_map = Texture2D::Create(materialValues["specular_map_path"].as<std::string>());
							}
							else {
								mat.specular_map = white_tex;
							}
							if (materialValues["normal_map_path"].as<std::string>() != "") {
								mat.normal_map = Texture2D::Create(materialValues["normal_map_path"].as<std::string>());
							}
							else {
								mat.normal_map = white_tex;
							}

							mshcmp.materials.push_back(mat);
						}
					}
				}

			}
		}

		auto cubemapPath = data["CubemapPath"];
		if (cubemapPath) {
			m_scene->setSkyBox(cubemapPath.as<std::string>());
		}

		return true;
	}

	bool SceneSerializer::deserializeRunTime(const std::string& path) {
		IARA_CORE_ASSERT(false);
		return false;
	}

}