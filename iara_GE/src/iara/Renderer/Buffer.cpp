#include "ir_pch.h"
#include "Buffer.h"

#include "Renderer.h"
#include "platform/openGL/OpenGLBuffer.h"

namespace iara {

    Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return CreateRef<OpenGLVertexBuffer>(vertices, size);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
    }

    Ref<VertexBuffer> VertexBuffer::Create(uint32_t size) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return CreateRef<OpenGLVertexBuffer>(size);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
    }

    Ref<VertexBuffer> VertexBuffer::Create() {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return CreateRef<OpenGLVertexBuffer>();
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
    }

    Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return CreateRef<OpenGLIndexBuffer>(indices, count);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
    }

    Ref<IndexBuffer> IndexBuffer::Create() {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return CreateRef<OpenGLIndexBuffer>();
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
    }

}