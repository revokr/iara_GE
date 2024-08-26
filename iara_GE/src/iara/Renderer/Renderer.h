#pragma once

#include "RenderCommand.h"
#include "Camera.h"
#include "shader.h"

namespace iara {

	class Renderer {
	public:
		static void BeginScene(PerspectiveCamera& camera);
		static void EndScene();

		static void Submit(const std::shared_ptr<Shader>& shdr, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

		inline static RendererAPI::API getRendererAPI() { return RendererAPI::getAPI(); }	
	private:
		struct SceneData {
			glm::mat4 ViewProj;
		};

		static SceneData* s_sceneData;
	};
}