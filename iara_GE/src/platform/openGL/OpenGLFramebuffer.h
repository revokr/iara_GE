#pragma once

#include "iara/Renderer/Framebuffer.h"
#include <glad/glad.h>

namespace iara {

	class OpenGLFrameBuffer : public Framebuffer {
	public:
		OpenGLFrameBuffer(const FramebufferSpecification& frame_buf_specs);
		virtual ~OpenGLFrameBuffer() override;

		virtual void bind() override;
		virtual void unbind() override;
		virtual void resize(uint32_t w, uint32_t h) override;
		virtual const FramebufferSpecification& getSpecification() const override {
			return m_specs;
		}

		virtual uint32_t getColorAtt() const override { return m_color_attachment; }
		virtual uint32_t getDepthAtt() const override { return m_depth_attachment; }
		virtual uint32_t getRendererID() const  override{ return m_RendererID; }

		void invalidate();
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_color_attachment = 0;
		uint32_t m_depth_attachment = 0;
		uint32_t m_rbo;
		FramebufferSpecification m_specs;
	};

}