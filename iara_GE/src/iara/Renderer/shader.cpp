#include "ir_pch.h"
#include "shader.h"
#include "Renderer.h"

#include "glad/glad.h"
#include "platform/openGL/OpenGLShader.h"

namespace iara {

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertSrc, const std::string& fragSrc) {
        switch (Renderer::getRendererAPI()) {
        case RendererAPI::API::None:     IARA_CORE_ASSERT(false, "RendererAPI::None is not supported!!");
        case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLShader>(name, vertSrc, fragSrc);
        }

        IARA_CORE_ASSERT(false, "Unknown renderer API");
        return nullptr;
	}

    void ShaderLibrary::addShader(const Ref<Shader>& s) {
        auto& name = s->getName();
        IARA_CORE_ASSERT((m_shaders.find(s) == m_shaders.end()), "Shader already exists!!");
        m_shaders[name] = s;
    }

    Ref<Shader> ShaderLibrary::load(const std::string& name, const std::string& vert_src, const std::string& frag_src) {
        auto shader = Shader::Create(name, vert_src, frag_src);
        addShader(shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::get(const std::string& name)
    {
        IARA_CORE_ASSERT((m_shaders.find(s) != m_shaders.end()), "Shader not found!!");
        return m_shaders[name];
    }

}