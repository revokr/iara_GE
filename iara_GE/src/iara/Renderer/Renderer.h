#pragma once

#include "RenderCommand.h"
#include "Camera.h"
#include "shader.h"

namespace iara {

	class Renderer {
	public:
		static void Init();
		static void onWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(/*OrthographicCamera*/PerspectiveCamera& camera);
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void Submit(const Ref<Shader>& shdr, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

		inline static RendererAPI::API getRendererAPI() { return RendererAPI::getAPI(); }	
	private:
		struct SceneData {
			glm::mat4 ViewProj;
		};

		static SceneData* s_sceneData;
	};
}