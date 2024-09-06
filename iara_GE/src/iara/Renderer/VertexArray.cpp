#include "ir_pch.h"
#include "VertexArray.h"

#include "Renderer.h"
#include "platform/openGL/OpenGLVertexArray.h"

namespace iara {

	Ref<VertexArray> VertexArray::Create() {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLVertexArray>();
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
	}

}