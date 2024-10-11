#pragma once

#include "Scene.h"

namespace iara {

	class SceneSerializer {
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void serialize(const std::string& path);
		void serializeRunTime(const std::string& path);

		bool deserialize(const std::string& path);
		bool deserializeRunTime(const std::string& path);
	private:
		Ref<Scene> m_scene;
	};

}