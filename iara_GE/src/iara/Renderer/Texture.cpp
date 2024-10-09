#include "ir_pch.h"
#include "Texture.h"

#include "Renderer.h"
#include "platform/openGL/OpenGLTexture.h"

namespace iara {
    Ref<Texture2D> Texture2D::CreateRef(uint32_t width, uint32_t height) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLTexture2D>(width, height);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
    }


    Ref<Texture2D> Texture2D::CreateRef(const std::string& path) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLTexture2D>(path);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
	}

}