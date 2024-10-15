#include "ir_pch.h"
#include "ContentBrowserPanel.h"

#include <imgui\imgui.h>

namespace iara {

	/// Will change this to represent the assets folder of a project
	extern const std::filesystem::path g_assets_path = "Assets";

	ContentBrowserPanel::ContentBrowserPanel() 
		: m_current_directory{ g_assets_path }
	{
		m_file_icon = Texture2D::CreateRef("Assets/Textures/icon2.png");
		m_dir_icon  = Texture2D::CreateRef("Assets/Textures/dir.png");
	}

	void ContentBrowserPanel::onImGuiRender() {
		ImGui::Begin("Browser Panel");

		if (m_current_directory != g_assets_path) {
			if (ImGui::Button("<-")) {
				m_current_directory = m_current_directory.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnail_size = 128.0f;
		float cell_size = thumbnail_size + padding;

		float panel_width = ImGui::GetContentRegionAvail().x;
		int column_count = (int)(panel_width / cell_size);
		if (column_count < 1) column_count = 1;

		ImGui::Columns(column_count, 0, false);

		for (auto& dir_entry : std::filesystem::directory_iterator(m_current_directory)) {
			const auto& path = dir_entry.path();
			auto relative_path = std::filesystem::relative(path, g_assets_path);
			std::string filename_string = path.filename().string();

			Ref<Texture2D> icon = dir_entry.is_directory() ? m_dir_icon : m_file_icon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton(filename_string.c_str(), (ImTextureID)icon->getRendererID(), { thumbnail_size, thumbnail_size }, { 0, 1 }, { 1, 0 });
			
			if (ImGui::BeginDragDropSource()) {
				const wchar_t* item_path = relative_path.c_str();
				ImGui::SetDragDropPayload("content_browser_item", item_path, wcslen(item_path) * sizeof(wchar_t) + 2);
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (dir_entry.is_directory()) {
					m_current_directory /= path.filename();
				}
			}
			ImGui::TextWrapped(filename_string.c_str());

			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnail_size, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		ImGui::End();
	}

}
