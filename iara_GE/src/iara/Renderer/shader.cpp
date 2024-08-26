#include "ir_pch.h"
#include "shader.h"

#include "glad/glad.h"

namespace iara {

	shader_prog_src Shader::parse_shader(const std::string& file_path_v, const std::string& file_path_f) {
		std::stringstream ss[2];
		std::string line;

		std::ifstream stream(file_path_v);
		while (getline(stream, line)) {
			ss[0] << line << '\n';
		}

		std::ifstream stream2(file_path_f);
		while (getline(stream2, line)) {
			ss[1] << line << '\n';
		}

		return { ss[0].str(), ss[1].str() };
	}

	unsigned int Shader::create_shader(const std::string& vertex_shader, const std::string& fragment_shader) {
		unsigned int prog = glCreateProgram();
		unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);
		unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);
		glAttachShader(prog, vs);
		glAttachShader(prog, fs);
		glLinkProgram(prog);
		glValidateProgram(prog);

		glDeleteShader(vs);
		glDeleteShader(fs);

		return prog;
	}

	unsigned int Shader::compile_shader(unsigned int type, const std::string& source) {

		unsigned int id = glCreateShader(type);
		const char* src = source.c_str(); // &source[0]
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		int result;
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);
		if (result == false) {
			int length;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
			char* message = (char*)alloca(length * sizeof(char));
			glGetShaderInfoLog(id, length, &length, message);

			std::cout << "Failed to compile " <<
				(type == GL_VERTEX_SHADER ? "vertex" : "fragment")
				<< " shader!!\n";
			std::cout << message << '\n';
		}
		return id;
	}


	Shader::Shader(const std::string& filepath_v, const std::string& filepath_f) : m_filepath_vert{ filepath_v },
		m_filepath_frag{filepath_f}
	{
		shader_prog_src src = parse_shader(filepath_v, filepath_f);
		//std::cout << src.vertex_source << '\n';
		//std::cout << src.fragment_source << '\n';
		m_RendererID = create_shader(src.vertex_source, src.fragment_source);
	}

	Shader::~Shader() {
		glDeleteProgram(m_RendererID);
	}

	void Shader::bind() const {
		glUseProgram(m_RendererID);
	}

	void Shader::unbind() const {
		glUseProgram(0);
	}
	
	int Shader::getUniformLocation(const std::string& name) const {
		if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
			return m_UniformLocationCache[name];
		}

		int location = glGetUniformLocation(m_RendererID, name.c_str());
		m_UniformLocationCache[name] = location;
		return location;
	}

	void Shader::setUniformMat4f(const std::string& name, const glm::mat4& matrix) {
		glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
	}

	void Shader::setUniform4f(const std::string& name,
		const glm::vec4& vec) {
		glUniform4f(getUniformLocation(name), vec.r, vec.g, vec.b, vec.a);
	}

}