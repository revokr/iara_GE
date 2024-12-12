#include "ir_pch.h"
#include "SceneHierarchyPanel.h"
#include "iara\Core\Input.h"

#include <glm\gtc\type_ptr.hpp>
#include <imgui\imgui.h>
#include <imgui\imgui_internal.h>

#include <iara\Math\Math.h>
#include <filesystem>
namespace iara {


	extern const std::filesystem::path g_assets_path;

	const ImGuiTreeNodeFlags imgui_treenode_flags = ImGuiTreeNodeFlags_DefaultOpen |
		ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context) {
		setContext(context);
	}

	void SceneHierarchyPanel::setContext(const Ref<Scene>& context) {
		m_context = context;
		m_selection_context = {};
	}

	void SceneHierarchyPanel::onImGuiRender() {
		ImGui::Begin("Scene Hierarchy");

		for (auto entityID : m_context->m_registry.view<entt::entity>()) {
			Entity entity{ entityID, m_context.get() };
			drawEntityNode(entity);
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_selection_context = {};

		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {

			if (ImGui::MenuItem("Create Empty Entity")) {
				m_context->createEntity("Empty Entity");
			}
			if (ImGui::MenuItem("Create Point Light")) {
				m_context->createPointLight("Point Light");
			}
			if (ImGui::MenuItem("Create Directional Light")) {
				m_context->createDirLight("Sky Light");
			}

			ImGui::EndPopup();
		}

		ImGui::End();


		ImGui::Begin("Properties");

		if (m_selection_context) {
			drawComponents(m_selection_context);
		}

		ImGui::End();
	}

	Entity SceneHierarchyPanel::setMouseHovered(uint32_t entityID) {
		Entity ent{ (entt::entity)entityID, m_context.get() };
		m_selection_context = ent;
		return ent;
	}

	void SceneHierarchyPanel::drawEntityNode(Entity entity) {
		auto& tag = entity.getComponent<TagComponent>().tag;
		
		ImGuiTreeNodeFlags flags = ((m_selection_context == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanFullWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked()) {
			m_selection_context = entity;
		}

		bool entity_deleted = false;
		if (ImGui::BeginPopupContextItem()) {

			if (ImGui::MenuItem("Delete Entity"))
				entity_deleted = true;

			ImGui::EndPopup();
		}

		if (opened) {
			ImGui::TreePop();
		}

		if (entity_deleted) {
			if (m_selection_context.hasComponent<PointLightComponent>()) m_context->decreasePointLights();
			m_context->destroyEntity(entity);
			if (m_selection_context == entity) {
				m_selection_context = {};
			}
		}
	}

	template<typename T, typename UIFunction>
	static void drawComponent(const std::string& name, Entity entity, UIFunction function) {
		if (entity.hasComponent<T>()) {
			auto& component = entity.getComponent<T>();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 1 });
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), imgui_treenode_flags, name.c_str());

			bool remove_comp = false;
			if (ImGui::BeginPopupContextItem()) {

				if (ImGui::MenuItem("Delete Component"))
					remove_comp = true;

				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			if (open) {
				function(component);

				ImGui::TreePop();
			}

			if (remove_comp) {
				entity.deleteComponent<T>();
			}
		}
	}

	static void drawVec3Control(const std::string& label, glm::vec3& values, float reset_val = 0.0f, float column_width = 70.0f) {
		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, column_width);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing | ImGuiStyleVar_FramePadding, ImVec2{ 0, 0 });

		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 button_size = { line_height + 3.0f, line_height - 0.5f };
		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		if (ImGui::Button("X", button_size)) {
			values.x = reset_val;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.8f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.9f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.8f, 0.15f, 1.0f));
		if (ImGui::Button("Y", button_size)) {
			values.y = reset_val;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.2f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.2f, 0.8f, 1.0f));
		if (ImGui::Button("Z", button_size)) {
			values.z = reset_val;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		
		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	void SceneHierarchyPanel::drawComponents(Entity entity) {
		if (entity.hasComponent<TagComponent>()) {
			auto& tag = entity.getComponent<TagComponent>().tag;
			
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component")) {
			ImGui::OpenPopup("AddComp");
		}

		if (ImGui::BeginPopup("AddComp")) {

			if (ImGui::MenuItem("Camera")) {
				m_selection_context.addComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Sprite")) {
				m_selection_context.addComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Texture")) {
				m_selection_context.addComponent<Texture2DComponent>("");

				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Cube3D")) {
				m_selection_context.addComponent<cube3DComponent>();

				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("PointLight")) {
				m_selection_context.addComponent<PointLightComponent>();

				m_context->increasePointLights();

				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("SkyLight")) {
				m_selection_context.addComponent<DirLightComponent>();

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		drawComponent<TransformComponent>("Transform", entity, [](auto& component) {
			drawVec3Control("Position", component.translation);
			glm::vec3 rot = glm::degrees(component.rotation);
			drawVec3Control("Rotation", rot);
			component.rotation = glm::radians(rot);
			drawVec3Control("Scale", component.scale, 1.0f);
		});

		drawComponent<CameraComponent>("Camera", entity, [](auto& component) {
			auto& camera = component.camera;

			ImGui::Checkbox("Primary", &component.primary);

			const char* cameraProjTypes[] = { "Perspective", "Orthographic" };
			const char* currentProjType = cameraProjTypes[(int)camera.getProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjType)) {
				for (int i = 0; i < 2; i++) {
					bool is_selected = cameraProjTypes[i] == currentProjType;
					if (ImGui::Selectable(cameraProjTypes[i], is_selected)) {
						currentProjType = cameraProjTypes[i];
						camera.setProjectionType((uint32_t)i);
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}


			if (camera.getProjectionType() == SceneCamera::ProjectionType::Orthographic) {
				float ortho_size = camera.getOrthographicSize();
				if (ImGui::DragFloat("Size", &ortho_size, 0.2f)) camera.setOrthographicSize(ortho_size);

				float ortho_near = camera.getOrthoNearClip();
				if (ImGui::DragFloat("NearClip", &ortho_near, 0.2f)) camera.setOrthoNearClip(ortho_near);

				float ortho_far = camera.getOrthoFarClip();
				if (ImGui::DragFloat("FarClip", &ortho_far, 0.2f)) camera.setOrthoFarClip(ortho_far);
			}

			if (camera.getProjectionType() == SceneCamera::ProjectionType::Perspective) {
				float perspective_fov = camera.getPerspectiveVerticalFov();
				if (ImGui::DragFloat("Vertical FOV", &perspective_fov, 0.2f)) camera.setPerspectiveVerticalFov(perspective_fov);

				float perspective_near = camera.getPerspectiveNearClip();
				if (ImGui::DragFloat("NearClip", &perspective_near, 0.2f)) camera.setPerspectiveNearClip(perspective_near);

				float perspective_far = camera.getPerspectiveFarClip();
				if (ImGui::DragFloat("FarClip", &perspective_far, 0.2f)) camera.setPerspectiveFarClip(perspective_far);
			}
		});

		drawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component) {
			ImGui::ColorEdit4("Color", &component.color.x);

			ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
			if (ImGui::BeginDragDropTarget()) {
				/// this payload can be null, that's why it's going through a check
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("content_browser_item")) {
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::string iara = std::filesystem::path(path).extension().string();

					std::filesystem::path tex_path = g_assets_path / std::filesystem::path(path);
					if (iara == ".png") {
						component.texture = Texture2D::Create(tex_path.string());
						component.tex_path = tex_path.string();
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::DragFloat("Tiling Factor", &component.tiling_factor, 0.1f, 0.0f, 100.0f);
		});

		drawComponent<cube3DComponent>("Cube3D", entity, [](auto& component) {
			ImGui::ColorEdit4("Color", &component.color.x);
			/*ImGui::ColorEdit4("Ambient", &component.material.ambient.x);
			ImGui::ColorEdit4("Diffuse", &component.material.diffuse.x);
			ImGui::ColorEdit4("Specular", &component.material.specular.x);
			ImGui::SliderFloat("Shininess", &component.material.shininess, 0.1f, 100.0f);*/
			ImGui::InputInt("Material Index", &component.material_index);

			ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
			if (ImGui::BeginDragDropTarget()) {
				/// this payload can be null, that's why it's going through a check
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("content_browser_item")) {
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::string iara = std::filesystem::path(path).extension().string();

					std::filesystem::path tex_path = g_assets_path / std::filesystem::path(path);
					if (iara == ".png") {
						component.texture = Texture2D::Create(tex_path.string());
						component.filepath = tex_path.string();
					}
				}
				ImGui::EndDragDropTarget();
			}
		});

		drawComponent<Texture2DComponent>("Texture", entity, [&](auto& component) {
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), component.filepath.c_str());

			ImGui::InputText("##Tag", buffer, sizeof(buffer));
			component.filepath = std::string(buffer);
			if (ImGui::Button("create")) {
				component.texture = Texture2D::Create(std::string(buffer));
			}
		});

		drawComponent<PointLightComponent>("Point Light", entity, [&](auto& component) {
			ImGui::ColorEdit4("Ambient", &component.plight.ambient.x);
			ImGui::ColorEdit4("Diffuse", &component.plight.diffuse.x);
			ImGui::ColorEdit4("Specular", &component.plight.specular.x);
			ImGui::SliderFloat("Constant", &component.plight.constant, 0.00001f, 0.9999);
			ImGui::InputFloat("Linear", &component.plight.linear, 0.0001f, 0.1);
			ImGui::InputFloat("Quadratic", &component.plight.quadratic, 0.0001f);
		});

		drawComponent<DirLightComponent>("Sky Light", entity, [&](auto& component) {
			ImGui::ColorEdit4("Ambient", &component.dlight.ambient.x);
			ImGui::ColorEdit4("Diffuse", &component.dlight.diffuse.x);
			ImGui::ColorEdit4("Specular", &component.dlight.specular.x);
		});

	}

}