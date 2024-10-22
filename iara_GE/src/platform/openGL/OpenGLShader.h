#pragma once

#include <string>
#include "glm/glm.hpp"
#include "iara/Renderer/shader.h"

typedef unsigned int GLenum;

namespace iara {

	/*struct shader_prog_src {
		std::string vertex_source;
		std::string fragment_source;
	};

	class OpenGLShader : public Shader {
	public:
		OpenGLShader(const std::string& name, const std::string& filepath_vert, const std::string& filepath_frag);
		~OpenGLShader();

		virtual void bind() const override;
		virtual void unbind() const override;
		virtual const std::string getName() override;

		void setUniformMat4f(const std::string& name, const glm::mat4& matrix) override;
		void setUniform4f(const std::string& name,
			const glm::vec4& vec) override;
		void setUniformInt(const std::string& name, int v0) override;
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
	};*/

	class OpenGLShader : public Shader{
	public:
		OpenGLShader(const std::string& name, const std::string& filepath_vert, const std::string& filepath_frag);
		~OpenGLShader();

		virtual void bind() const override;
		virtual void unbind() const override;
		virtual const std::string getName() override { return m_name; };

		void setUniformMat4f(const std::string& name, const glm::mat4& matrix) override;
		void setUniform4f(const std::string& name,
			const glm::vec4& vec) override;
		void setUniformInt(const std::string& name, int v0) override;
		void setUniformIntArray(const std::string& name, int* v, uint32_t count) override;
		void setUniformFloat(const std::string& name, float val) override;
		void setUniformFloat3(const std::string& name, float val0, float val1, float val2) override;
	private:

		void Compile_or_GetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shader_sources);
		void Compile_or_GetOpenGLBinaries();
		void CreateProgram();

		void reflect(uint32_t stage, const std::vector<uint32_t>& shader_data);

		std::string readFile(const std::string& path);

	private:
		uint32_t m_RendererID;
		std::string m_filepath_vert;
		std::string m_filepath_frag;
		std::string m_name;
		
		std::unordered_map<GLenum, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>> m_OpenGLSPIRV;

		std::unordered_map<GLenum, std::string> m_OpenGL_src_code;
	};

}