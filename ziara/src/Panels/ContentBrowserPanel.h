#pragma once

#include "iara\Renderer\Texture.h"

#include <filesystem>

namespace iara {

	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();

		void onImGuiRender();
	private:
		std::filesystem::path m_current_directory;
		Ref<Texture2D> m_dir_icon;
		Ref<Texture2D> m_file_icon;
	};

}