#include "ir_pch.h"
#include "UniformBuffer.h"

#include "iara\Renderer\Renderer.h"
#include "platform\openGL\OpenGLUniformBuffer.h"

namespace iara {

    Ref<UniformBuffer> iara::UniformBuffer::Create(uint32_t size, uint32_t binding) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None: IARA_CORE_ASSERT(false);
        case RendererAPI::API::OpenGL:return CreateRef<OpenGLUniformBuffer>(size, binding);
        }

        return nullptr;
    }

}


