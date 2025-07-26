#pragma once

#include "iara/Renderer/Framebuffer.h"
#include <glad/glad.h>

namespace iara {

	class OpenGLMSAAFrameBuffer : public Framebuffer {
	public:
		OpenGLMSAAFrameBuffer(const FramebufferSpecification& frame_buf_specs);
		virtual ~OpenGLMSAAFrameBuffer() override;

		virtual void bind() override;
		virtual void unbind() override;
		virtual void resize(uint32_t w, uint32_t h) override;
		virtual const FramebufferSpecification& getSpecification() const override {
			return m_specs;
		}

		virtual int readPixel(uint32_t att_index, int x, int y) override;
		virtual void clearAttachment(uint32_t att_indx, int value) override;

		virtual uint32_t getColorAtt(uint32_t index = 0) const override { return m_specs.samples > 1 ? m_color_att_resolve : m_color_attachments[0]; }
		virtual uint32_t getDepthAtt() const override { return m_depth_attachment; }
		virtual uint32_t getRendererID() const  override{ return m_RendererID; }

		void invalidate();
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_ResolveFBO = 0;
		uint32_t m_RBO = 0;
		FramebufferSpecification m_specs;

		std::vector<FramebufferTextureSpecification> m_color_att_specs;
		FramebufferTextureSpecification m_depth_att_spec;

		std::vector<uint32_t> m_color_attachments;
		uint32_t m_depth_attachment;

		uint32_t m_color_att_resolve = 0;
		uint32_t m_color_att_resolve_single_channel = 0;
	};

	class OpenGLFrameBuffer : public Framebuffer {
	public:
		OpenGLFrameBuffer(const FramebufferSpecification& frame_buf_specs, const std::string& name = "");
		virtual ~OpenGLFrameBuffer() override;

		virtual void bind() override;
		virtual void unbind() override;
		virtual void resize(uint32_t w, uint32_t h) override;
		virtual const FramebufferSpecification& getSpecification() const override {
			return m_specs;
		}

		virtual int readPixel(uint32_t att_index, int x, int y) override;
		virtual void clearAttachment(uint32_t att_indx, int value) override;

		virtual uint32_t getColorAtt(uint32_t index = 0) const override { return  m_color_attachments[0]; }
		virtual uint32_t getDepthAtt() const override { return m_depth_attachment; }
		virtual uint32_t getRendererID() const  override { return m_RendererID; }

		void invalidate();
	private:
		std::string m_name = "";

		uint32_t m_RendererID = 0;
		uint32_t m_ResolveFBO = 0;
		uint32_t m_RBO = 0;
		FramebufferSpecification m_specs;

		std::vector<FramebufferTextureSpecification> m_color_att_specs;
		FramebufferTextureSpecification m_depth_att_spec;

		std::vector<uint32_t> m_color_attachments;
		uint32_t m_depth_attachment;

		uint32_t m_color_att_resolve = 0;
		uint32_t m_color_att_resolve_single_channel = 0;
	};

}