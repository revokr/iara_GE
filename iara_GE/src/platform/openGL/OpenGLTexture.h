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
		virtual const uint32_t getRendererID() const override { return m_RendererID; }

		virtual bool operator==(const Texture& other) const override {
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}
	private:
		std::string m_path;

		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_RendererID;
		GLenum m_internal_format, m_data_format;
	};

	class OpenGLCubeMapTexture : public Texture2D {
	public:
		OpenGLCubeMapTexture(const std::vector<std::string>& path);
		OpenGLCubeMapTexture(const OpenGLCubeMapTexture& other);
		virtual ~OpenGLCubeMapTexture();

		virtual void bind(uint32_t slot = 0) const override;
		virtual void unbind() const override;

		virtual void setData(void* data, uint32_t size) override;

		virtual const uint32_t getWidth() const override { return 0; }
		virtual const uint32_t getHeight() const override { return 0; }
		virtual const uint32_t getRendererID() const override { return m_RendererID; }

		virtual bool operator==(const Texture& other) const override {
			return m_RendererID == ((OpenGLCubeMapTexture&)other).m_RendererID;
		}
	private:
		std::vector<std::string> m_paths;

		uint32_t m_RendererID;
	};

}