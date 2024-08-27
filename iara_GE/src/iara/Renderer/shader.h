#pragma once

#include <string>
#include "glm/glm.hpp"

typedef struct shader_prog_src {
	std::string vertex_source;
	std::string fragment_source;
};

namespace iara {

	class Shader {
	public:
		virtual ~Shader() = default;

		virtual void bind() const   = 0;
		virtual void unbind() const = 0;
	
		static Shader* Create(const std::string& vertSrc, const std::string& fragSrc);
	private:
		uint32_t m_RendererID;
	};

}