#include "ir_pch.h"
#include "OpenGLTexture.h"

#include <stb_image.h>

namespace iara {

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_width{width}, m_height{height}
	{
		IARA_PROFILE_FUNCTION();

		m_internal_format = GL_RGBA8;
		m_data_format = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_internal_format, m_width, m_height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_path{path}
	{
		IARA_PROFILE_FUNCTION();

		stbi_set_flip_vertically_on_load(1);
		int width, height, channels;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		IARA_CORE_ASSERT(data, "Failed to load Image!!");

		m_width = width;
		m_height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4) {
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3) {
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_width, m_height);

		glTexParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_width, m_height, dataFormat, GL_UNSIGNED_BYTE, data);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D() {
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::bind(uint32_t slot) const {
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture2D::unbind() const {
		glBindTextureUnit(0, 0);
	}

	void OpenGLTexture2D::setData(void* data, uint32_t size) {
		uint32_t bpp = m_data_format == GL_RGBA ? 4 : 3;
		IARA_CORE_ASSERT(size == m_width * m_height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_width, m_height, m_data_format, GL_UNSIGNED_BYTE, data);
	}

	OpenGLCubeMapTexture::OpenGLCubeMapTexture(const std::vector<std::string>& faces)
		: m_paths{ faces } {
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

		int width, height, nrChannels;
		for (uint32_t i = 0; i < faces.size(); i++) {
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else {
				IARA_CORE_ERROR("Failed to load texture at path {0}", faces[i]);
				stbi_image_free(data);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	OpenGLCubeMapTexture::OpenGLCubeMapTexture(const OpenGLCubeMapTexture& other) {
		auto new_tex = OpenGLCubeMapTexture(other.m_paths);
		m_RendererID = new_tex.m_RendererID;
	}

	OpenGLCubeMapTexture::~OpenGLCubeMapTexture() {
		glDeleteTextures(1, &m_RendererID);
		m_paths.clear();
	}

	void OpenGLCubeMapTexture::bind(uint32_t slot) const {
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
	}

	void OpenGLCubeMapTexture::unbind() const {
		glBindTextureUnit(0, 0);
	}

	void OpenGLCubeMapTexture::setData(void* data, uint32_t size)
	{
	}

}