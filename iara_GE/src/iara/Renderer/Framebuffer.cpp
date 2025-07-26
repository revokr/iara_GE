#include "ir_pch.h"
#include "Framebuffer.h"
#include "iara/Renderer/Renderer.h"

#include "platform/openGL/OpenGLFramebuffer.h"

namespace iara {

    Ref<Framebuffer> Framebuffer::CreateMSAA(const FramebufferSpecification& frame_buf_specs) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return CreateRef<OpenGLMSAAFrameBuffer>(frame_buf_specs);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
    }

    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& frame_buf_specs) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return CreateRef<OpenGLFrameBuffer>(frame_buf_specs);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
    }

    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& frame_buf_specs, const std::string& name) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return CreateRef<OpenGLFrameBuffer>(frame_buf_specs, name);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
    }

}
