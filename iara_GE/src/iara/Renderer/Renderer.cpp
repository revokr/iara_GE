#include "ir_pch.h"
#include "Renderer.h"

namespace iara {

    Renderer::SceneData* Renderer::s_sceneData = new Renderer::SceneData;

    void Renderer::BeginScene(PerspectiveCamera& camera) {
        s_sceneData->ViewProj = camera.getVP();
    }

    void Renderer::EndScene() {

    }

    void Renderer::Submit(const std::shared_ptr<Shader>& shdr, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform) {
        shdr->bind();
        shdr->setUniformMat4f("u_VP", s_sceneData->ViewProj);
        shdr->setUniformMat4f("u_transform", transform);

        vertexArray->bind();
        RenderCommand::DrawIndexed(vertexArray);
    }

}
