#include "ir_pch.h"
#include "Renderer.h"

namespace iara {

    Renderer::SceneData* Renderer::s_sceneData = new Renderer::SceneData;

    void Renderer::BeginScene(OrthographicCamera& camera) {
        s_sceneData->ViewProj = camera.getVP();
    }

    void Renderer::EndScene() {

    }

    void Renderer::Submit(const std::shared_ptr<Shader>& shdr, const std::shared_ptr<VertexArray>& vertexArray) {
        shdr->bind();
        shdr->setUniformMat4f("u_MVP", s_sceneData->ViewProj);

        vertexArray->bind();
        RenderCommand::DrawIndexed(vertexArray);
    }

}
