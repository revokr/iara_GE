#include "ir_pch.h"
#include "Buffer.h"

#include "Renderer.h"
#include "platform/openGL/OpenGLBuffer.h"

namespace iara {

    VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return new OpenGLVertexBuffer(vertices, size);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
    }

    IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t size) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return new OpenGLIndexBuffer(indices, size);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
    }

}