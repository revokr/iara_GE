#pragma once

#include <string>
#include "glm/glm.hpp"
#include "iara/Renderer/shader.h"

namespace iara {

	struct shader_prog_src {
		std::string vertex_source;
		std::string fragment_source;
	};

	class OpenGLShader : public Shader{
	public:
		OpenGLShader(const std::string& name, const std::string& filepath_vert, const std::string& filepath_frag);
		~OpenGLShader();

		virtual void bind() const override;
		virtual void unbind() const override;
		virtual const std::string getName() override;

		void setUniformMat4f(const std::string& name, const glm::mat4& matrix) override;
		void setUniform4f(const std::string& name,
			const glm::vec4& vec) override;
		void setUniformInt(const std::string& name,int v0) override;
		void setUniformIntArray(const std::string& name, int* v, uint32_t count) override;
		void setUniformFloat(const std::string& name, float val) override;
	private:
		int getUniformLocation(const std::string& name) const;
		unsigned int compile_shader(unsigned int type, const std::string& source);
		unsigned int create_shader(const std::string& vertex_shader, const std::string& fragment_shader);
		shader_prog_src parse_shader(const std::string& file_path_v, const std::string& file_path_f);
	private:
		std::string m_filepath_vert, m_filepath_frag, m_name;
		uint32_t m_RendererID;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
	};

}