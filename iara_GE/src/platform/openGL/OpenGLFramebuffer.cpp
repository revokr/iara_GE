#include "ir_pch.h"
#include "OpenGLFramebuffer.h"


namespace iara {

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FramebufferSpecification& frame_buf_specs)
		: m_specs{frame_buf_specs}
	{
		invalidate();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer() {
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_color_attachment);
		glDeleteTextures(1, &m_depth_attachment);
	}

	void OpenGLFrameBuffer::invalidate() {

		if (m_RendererID) {
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(1, &m_color_attachment);
			glDeleteTextures(1, &m_depth_attachment);
		}

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		//IARA_CORE_WARN("FrameBuffer: {0}", m_RendererID);

		glGenTextures(1, &m_color_attachment);
		glBindTexture(GL_TEXTURE_2D, m_color_attachment);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_specs.width, m_specs.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_attachment, 0);

		//IARA_CORE_WARN("Color att: {0}", m_color_attachment);


		glGenTextures(1, &m_depth_attachment);
		glBindTexture(GL_TEXTURE_2D, m_depth_attachment);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_specs.width, m_specs.height);
		/*glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_specs.width, m_specs.height, 0,
			GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);*/
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_attachment, 0);

		//IARA_CORE_WARN("Depth att: {0}", m_depth_attachment);

		int check = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		//IARA_CORE_ERROR("Framebuffer Status: {0} == {1}", check, GL_FRAMEBUFFER_COMPLETE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_specs.width, m_specs.height);
	}

	void OpenGLFrameBuffer::unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDisable(GL_DEPTH_TEST);
	}
	void OpenGLFrameBuffer::resize(uint32_t w, uint32_t h) {
		m_specs.width = w;
		m_specs.height = h;

		invalidate();
	}
}

