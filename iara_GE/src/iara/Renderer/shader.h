#pragma once

#include <string>
#include <unordered_map>
#include "glm/glm.hpp"

namespace iara {

	class Shader {
	public:
		virtual ~Shader() = default;

		virtual void bind() const   = 0;
		virtual void unbind() const = 0;
		virtual const std::string getName() = 0;
		
		virtual void setUniformMat4f(const std::string& name, const glm::mat4& mat) = 0;
		virtual void setUniform4f(const std::string& name, const glm::vec4& value) = 0;
		virtual void setUniformInt(const std::string& name, int value) = 0;
		virtual void setUniformIntArray(const std::string& name, int* v, uint32_t count) = 0;
		virtual void setUniformFloat(const std::string& name, float val) = 0;
	
		static Ref<Shader> Create(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);
	private:
		uint32_t m_RendererID;
	};

	class ShaderLibrary {
	public:
		void addShader(const Ref<Shader>& s);
		Ref<Shader> load(const std::string& name, const std::string& vert_src, const std::string& frag_src);
		Ref<Shader> get(const std::string& name);
	private:
		std::unordered_map<std::string, Ref<Shader>> m_shaders;
	};

}