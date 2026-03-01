#pragma once

#include "iara\Renderer\Renderer.h"

namespace iara {
	
	class SceneRenderer {
	public:
		//static void renderScene(iara::Timestep ts, EditorCamera& editor_camera);

		//static void setScene(const Ref<Scene>& scene) { m_scene = scene; }

	private:
		Ref<Scene> m_scene;
	};
}