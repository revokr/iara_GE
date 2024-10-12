#include "ir_pch.h"
#include "platform/openGL/OpenGLShader.h"
#include "iara\Core\Timer.h"

#include <fstream>
#include <filesystem>
#include <glad\glad.h>

#include <shaderc\shaderc.hpp>
#include <spirv_cross\spirv_cross.hpp>
#include <spirv_cross\spirv_glsl.hpp>

namespace iara {

	//shader_prog_src OpenGLShader::parse_shader(const std::string& file_path_v, const std::string& file_path_f) {
	//	std::stringstream ss[2];
	//	std::string line;

	//	std::ifstream stream(file_path_v);
	//	while (getline(stream, line)) {
	//		ss[0] << line << '\n';
	//	}

	//	std::ifstream stream2(file_path_f);
	//	while (getline(stream2, line)) {
	//		ss[1] << line << '\n';
	//	}

	//	return { ss[0].str(), ss[1].str() };
	//}

	//unsigned int OpenGLShader::create_shader(const std::string& vertex_shader, const std::string& fragment_shader) {
	//	unsigned int prog = glCreateProgram();
	//	unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);
	//	unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);
	//	glAttachShader(prog, vs);
	//	glAttachShader(prog, fs);
	//	glLinkProgram(prog);
	//	glValidateProgram(prog);

	//	glDeleteShader(vs);
	//	glDeleteShader(fs);

	//	return prog;
	//}

	//unsigned int OpenGLShader::compile_shader(unsigned int type, const std::string& source) {

	//	unsigned int id = glCreateShader(type);
	//	const char* src = source.c_str(); // &source[0]
	//	glShaderSource(id, 1, &src, nullptr);
	//	glCompileShader(id);

	//	int result;
	//	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	//	if (result == false) {
	//		int length;
	//		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
	//		char* message = (char*)alloca(length * sizeof(char));
	//		glGetShaderInfoLog(id, length, &length, message);

	//		std::cout << "Failed to compile " <<
	//			(type == GL_VERTEX_SHADER ? "vertex" : "fragment")
	//			<< " shader!!\n";
	//		std::cout << message << '\n';
	//	}
	//	return id;
	//}


	//OpenGLShader::OpenGLShader(const std::string& name, const std::string& filepath_v, const std::string& filepath_f) : m_filepath_vert{ filepath_v },
	//	m_filepath_frag{ filepath_f }, m_name{ name }
	//{
	//	IARA_PROFILE_FUNCTION();

	//	shader_prog_src src = parse_shader(filepath_v, filepath_f);
	//	//std::cout << src.vertex_source << '\n';
	//	//std::cout << src.fragment_source << '\n';
	//	m_RendererID = create_shader(src.vertex_source, src.fragment_source);
	//}

	//const std::string OpenGLShader::getName() {
	//	return m_name;
	//}

	//int OpenGLShader::getUniformLocation(const std::string& name) const {
	//	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
	//		return m_UniformLocationCache[name];
	//	}

	//	int location = glGetUniformLocation(m_RendererID, name.c_str());
	//	m_UniformLocationCache[name] = location;
	//	return location;
	//}

	namespace utils {

		static GLenum shaderTypeFromString(const std::string& type) {
			if (type == "vertex") {
				return GL_VERTEX_SHADER;
			}
			else if (type == "fragment") {
				return GL_FRAGMENT_SHADER;
			}

			return 0;
		}

		static const char* getCacheDir() {
			return "Assets/cahce/shader/opengl";
		}

		static const char* getShaderStageExtensionOpenGL(uint32_t type) {
			switch (type)
			{
			case GL_VERTEX_SHADER:   return ".cached_opengl.vert";
			case GL_FRAGMENT_SHADER: return ".cached_opengl.frag";
			}

			return "";
		}

		static const char* getShaderStageExtensionVulkan(uint32_t type) {
			switch (type)
			{
			case GL_VERTEX_SHADER:   return ".cached_vulkan.vert";
			case GL_FRAGMENT_SHADER: return ".cached_vulkan.frag";
			}

			return "";
		}

		static const char* glShaderToString(uint32_t type) {
			switch (type)
			{
			case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			}

			return "";
		}

		static shaderc_shader_kind getShaderCTypeFromGLSL(uint32_t stage) {
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			}

			return (shaderc_shader_kind)0;
		}

		static void createCacheDirIfNeeded() {
			std::string cacheDir = getCacheDir();
			if (!std::filesystem::exists(cacheDir)) {
				std::filesystem::create_directories(cacheDir);
			}
		}

	}


	OpenGLShader::OpenGLShader(const std::string& name, const std::string& filepath_v, const std::string& filepath_f) 
		: m_filepath_vert{ filepath_v }, m_filepath_frag{ filepath_f }, m_name{name}
	{
		utils::createCacheDirIfNeeded();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = readFile(m_filepath_vert);
		sources[GL_FRAGMENT_SHADER] = readFile(m_filepath_frag);

		{
			Timer timer;
			Compile_or_GetVulkanBinaries(sources);
			Compile_or_GetOpenGLBinaries();
			CreateProgram();
			IARA_CORE_WARN("Shader creation took {0} ms", timer.elapsedMilliseconds());
		}
	}

	void OpenGLShader::Compile_or_GetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shader_sources) {
		/// idk what this line is for
		GLuint program = glCreateProgram();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize = true;
		if (optimize) {
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		std::filesystem::path cache_dir = utils::getCacheDir();

		auto& shader_data = m_VulkanSPIRV;
		shader_data.clear();
		for (auto&& [stage, source] : shader_sources) {
			std::filesystem::path shader_file_path;
			if (stage == GL_VERTEX_SHADER) 
				shader_file_path = m_filepath_vert;
			else 
				shader_file_path = m_filepath_frag;

			std::filesystem::path cached_pathh = cache_dir;

			std::string cached_path = cached_pathh.string() + "/" + shader_file_path.filename().string() + utils::getShaderStageExtensionVulkan(stage);

			std::ifstream in(cached_path.c_str(), std::ios::in | std::ios::binary);
			if (in.is_open()) {
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shader_data[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else {
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, utils::getShaderCTypeFromGLSL(stage),
					stage == GL_VERTEX_SHADER ? m_filepath_vert.c_str() : m_filepath_frag.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
					IARA_CORE_ERROR(module.GetErrorMessage());
				}

				shader_data[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cached_path.c_str(), std::ios::out | std::ios::binary);
				if (out.is_open()) {
					auto& data = shader_data[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shader_data) {
			reflect(stage, data);
		}

	}

	void OpenGLShader::Compile_or_GetOpenGLBinaries() {
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

		const bool optimize = true;
		if (optimize) {
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		std::filesystem::path cache_dir = utils::getCacheDir();

		auto& shader_data = m_OpenGLSPIRV;
		shader_data.clear();
		m_OpenGL_src_code.clear();

		for (auto&& [stage, spirv] : m_VulkanSPIRV) {
			std::filesystem::path shader_file_path = stage == GL_VERTEX_SHADER ? m_filepath_vert : m_filepath_frag;

			std::filesystem::path cached_pathh = cache_dir;

			std::string cached_path = cached_pathh.string() + "/" + shader_file_path.filename().string() + utils::getShaderStageExtensionVulkan(stage);

			std::ifstream in(cached_path, std::ios::in | std::ios::binary);
			if (in.is_open()) {
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shader_data[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else {
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_OpenGL_src_code[stage] = glslCompiler.compile();
				auto& source = m_OpenGL_src_code[stage];

				
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, utils::getShaderCTypeFromGLSL(stage),
					stage == GL_VERTEX_SHADER ? m_filepath_vert.c_str() : m_filepath_frag.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
					IARA_CORE_ERROR(module.GetErrorMessage());
				}

				shader_data[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cached_path, std::ios::out | std::ios::binary);
				if (out.is_open()) {
					auto& data = shader_data[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGLShader::CreateProgram() {
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV) {
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE) {
			GLint max_len;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_len);

			std::vector<GLchar> infoLog(max_len);

			glGetProgramInfoLog(program, max_len, &max_len, infoLog.data());
			IARA_CORE_ERROR("Shader linking failed (:{1})", infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIDs) {
				glDeleteShader(id);
			}
		}

		for (auto id : shaderIDs) {
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	void OpenGLShader::reflect(uint32_t stage, const std::vector<uint32_t>& shader_data) {
		spirv_cross::Compiler compiler(shader_data);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		IARA_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", utils::glShaderToString(stage), stage == GL_VERTEX_SHADER ? m_filepath_vert : m_filepath_frag);
		IARA_CORE_TRACE("     {0} uniform buffers", resources.uniform_buffers.size());
		IARA_CORE_TRACE("     {0} resources", resources.sampled_images.size());

		IARA_CORE_TRACE("Uniform buffers: ");
		for (const auto& res : resources.uniform_buffers) {
			const auto& buffer_type = compiler.get_type(res.base_type_id);
			uint32_t buffer_size = (uint32_t)compiler.get_declared_struct_size(buffer_type);
			uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
			int member_count = (int)buffer_type.member_types.size();

			IARA_CORE_TRACE("   {0}", res.name);
			IARA_CORE_TRACE("    Size = {0}", buffer_size);
			IARA_CORE_TRACE("    Binding = {0}", binding);
			IARA_CORE_TRACE("    Members = {0}", member_count);
		}
	}

	std::string OpenGLShader::readFile(const std::string& path) {
		std::string result;
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1) {
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else {
				IARA_CORE_ERROR("Could not find filepath: {0}", path);
			}
		}
		else {
			IARA_CORE_ERROR("Could not open file: {0}", path);
		}

		return result;
	}

	OpenGLShader::~OpenGLShader() {
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::bind() const {
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::unbind() const {
		glUseProgram(0);
	}

	void OpenGLShader::setUniformMat4f(const std::string& name, const glm::mat4& matrix) {
		glUniformMatrix4fv(glGetUniformLocation(m_RendererID, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
	}

	void OpenGLShader::setUniform4f(const std::string& name,
		const glm::vec4& vec) {
		glUniform4f(glGetUniformLocation(m_RendererID, name.c_str()), vec.r, vec.g, vec.b, vec.a);
	}

	void OpenGLShader::setUniformInt(const std::string& name, int v0) {
		glUniform1i(glGetUniformLocation(m_RendererID, name.c_str()), v0);
	}

	void OpenGLShader::setUniformIntArray(const std::string& name, int* v, uint32_t count) {
		glUniform1iv(glGetUniformLocation(m_RendererID, name.c_str()), count, v);
	}

	void OpenGLShader::setUniformFloat(const std::string& name, float val) {
		glUniform1f(glGetUniformLocation(m_RendererID, name.c_str()), val);
	}

}