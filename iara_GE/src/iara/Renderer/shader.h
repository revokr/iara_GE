#pragma once

#include <string>
#include "glm/glm.hpp"
namespace iara {

	class Shader {
	public:
		Shader(const std::string& vertSrc, const std::string& fragSrc);
		~Shader();

		void bind() const;
		void unbind() const;

		int getUniformLocation(const std::string& name) const;
		void setUniformMat4f(const std::string& name, const glm::mat4& matrix);
	private:
		uint32_t m_RendererID;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
	};

}