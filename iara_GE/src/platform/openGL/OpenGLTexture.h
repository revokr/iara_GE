#pragma once

#include "iara/Renderer/Texture.h"
#include <glad/glad.h>

namespace iara {

	class OpenGLTexture2D : public Texture2D {
	public:
		OpenGLTexture2D(uint32_t wifth, uint32_t height);
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual void bind(uint32_t slot = 0) const override;
		virtual void unbind() const override;

		virtual void setData(void* data, uint32_t size) override;

		virtual const uint32_t getWidth() const override { return m_width; }
		virtual const uint32_t getHeight() const override { return m_height; }
	private:
		std::string m_path;

		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_RendererID;
		GLenum m_internal_format, m_data_format;
	};

}