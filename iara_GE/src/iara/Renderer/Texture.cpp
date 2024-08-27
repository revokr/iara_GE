#include "ir_pch.h"
#include "Texture.h"

#include "Renderer.h"
#include "platform/openGL/OpenGLTexture.h"

namespace iara {

	Ref<Texture2D> Texture2D::Create(const std::string& path) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLTexture2D>(path);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
	}

}