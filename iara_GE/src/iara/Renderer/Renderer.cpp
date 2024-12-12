#include "ir_pch.h"
#include "Renderer.h"
#include "platform/openGL/OpenGLShader.h"
#include "Renderer2D.h"

namespace iara {

    Renderer::SceneData* Renderer::s_sceneData = new Renderer::SceneData;

    void Renderer::Init() {
        RenderCommand::Init();
        Renderer2D::Init();
        Renderer3D::Init3D();
    }

    void Renderer::onWindowResize(uint32_t width, uint32_t height) {
        RenderCommand::setViewPort(0, 0, width, height);
    }

    void Renderer::BeginScene(PerspectiveCamera& camera) {
        s_sceneData->ViewProj = camera.getVP();
    }

    void Renderer::BeginScene(OrthographicCamera& camera) {
        s_sceneData->ViewProj = camera.getVP();
    }

    void Renderer::EndScene() {

    }

    void Renderer::Submit(const Ref<Shader>& shdr, const Ref<VertexArray>& vertexArray, const glm::mat4& transform) {
        std::dynamic_pointer_cast<OpenGLShader>(shdr)->bind();
        std::dynamic_pointer_cast<OpenGLShader>(shdr)->setUniformMat4f("u_VP", s_sceneData->ViewProj);
        std::dynamic_pointer_cast<OpenGLShader>(shdr)->setUniformMat4f("u_transform", transform);

        vertexArray->bind();
        RenderCommand::DrawIndexed(vertexArray);
    }

}
