#pragma once

#include <string>
#include "glm/glm.hpp"
#include "iara/Renderer/shader.h"

namespace iara {

	class OpenGLShader : public Shader{
	public:
		OpenGLShader(const std::string& filepath_vert, const std::string& filepath_frag);
		~OpenGLShader();

		void bind() const;
		void unbind() const;

		void setUniformMat4f(const std::string& name, const glm::mat4& matrix);
		void setUniform4f(const std::string& name,
			const glm::vec4& vec);
		void setUniformInt(const std::string& name,int v0);
	private:
		int getUniformLocation(const std::string& name) const;
		unsigned int compile_shader(unsigned int type, const std::string& source);
		unsigned int create_shader(const std::string& vertex_shader, const std::string& fragment_shader);
		shader_prog_src parse_shader(const std::string& file_path_v, const std::string& file_path_f);
	private:
		std::string m_filepath_vert, m_filepath_frag;
		uint32_t m_RendererID;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
	};

}