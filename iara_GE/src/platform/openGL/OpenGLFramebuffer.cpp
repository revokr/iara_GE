#include "ir_pch.h"
#include "OpenGLFramebuffer.h"


namespace iara {

	static uint32_t s_maxFramebufferSize = 2048;

	namespace utils {
		
		static GLenum textureTarget(bool multisample) {
			return multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void createTextures(bool multisample, uint32_t* outID, uint32_t count) {
			glCreateTextures(textureTarget(multisample), count, outID);
		}

		static void bindTexture(bool multisample, uint32_t id) {
			glBindTexture(textureTarget(multisample), id);
		}

		static void attachColorTexture(uint32_t id, int samples, GLenum internal_format, GLenum format, uint32_t width, uint32_t height, int index) {
			bool multisample = samples > 1;
			if (multisample) {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, textureTarget(multisample), id, 0);
		}

		static void attachDepthTexture(uint32_t id, int samples, GLenum format, GLenum att_type, uint32_t width, uint32_t height) {
			bool multisample = samples > 1;
			if (multisample) {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else {
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, att_type, textureTarget(multisample), id, 0);
		}

		static bool isDepthFormat(FramebufferTextureFormat format) {
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				return true;
			}

			return false;
		}

		static GLenum iaraFramebufferTexFormatToGL(FramebufferTextureFormat format) {
			switch (format)
			{
			case iara::FramebufferTextureFormat::RGBA8:
				return GL_RGBA8;
				break;
			case iara::FramebufferTextureFormat::RED_INTEGER:
				return GL_RED_INTEGER;
				break;
			}

			return 0;
		}

		/*static GLenum GLDataType(FramebufferTextureFormat format) {
			switch (format)
			{
			case iara::FramebufferTextureFormat::RGBA8:
				return GL_UNSIGNED_BYTE;
				break;
			case iara::FramebufferTextureFormat::RED_INTEGER:
				return GL_INT;
				break;
			}

			return 0;
		}*/

	}

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FramebufferSpecification& frame_buf_specs)
		: m_specs{frame_buf_specs}
	{
		for (auto spec : m_specs.attachments.attachments) {
			if (!utils::isDepthFormat(spec.texture_format)) {
				m_color_att_specs.emplace_back(spec);
			} else {
				m_depth_att_spec = spec;
			}
		}

		invalidate();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer() {
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures((GLsizei)m_color_attachments.size(), m_color_attachments.data());
		glDeleteTextures(1, &m_depth_attachment);
	}

	void OpenGLFrameBuffer::invalidate() {

		if (m_RendererID) {
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures((GLsizei)m_color_attachments.size(), m_color_attachments.data());
			glDeleteTextures(1, &m_depth_attachment);

			m_color_attachments.clear();
			m_depth_attachment = 0;
		}

		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool multi_sample = m_specs.samples > 1;

		/// Attachments
		if (m_color_att_specs.size()) {
			m_color_attachments.resize(m_color_att_specs.size());
			utils::createTextures(multi_sample, m_color_attachments.data(), (uint32_t)m_color_attachments.size());
			for (size_t i = 0; i < m_color_attachments.size(); i++) {
				utils::bindTexture(multi_sample, m_color_attachments[i]);
				switch (m_color_att_specs[i].texture_format)
				{
				case FramebufferTextureFormat::RGBA8:
					utils::attachColorTexture(m_color_attachments[i], m_specs.samples, GL_RGBA8, GL_RGBA, m_specs.width, m_specs.height, (uint32_t)i);
					break;
				case FramebufferTextureFormat::RED_INTEGER:
					utils::attachColorTexture(m_color_attachments[i], m_specs.samples, GL_R32I, GL_RED_INTEGER, m_specs.width, m_specs.height, (uint32_t)i);
					break;
				}
			}
		}

		if (m_depth_att_spec.texture_format != FramebufferTextureFormat::None) {
			utils::createTextures(multi_sample, &m_depth_attachment, 1);
			utils::bindTexture(multi_sample, m_depth_attachment);

			switch (m_depth_att_spec.texture_format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				utils::attachDepthTexture(m_depth_attachment, m_specs.samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_specs.width, m_specs.height);
			}
		}

		if (m_color_attachments.size() > 1) {
			//IARA_CORE_ASSERT(m_color_attachments.size() <= 4);
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers((GLsizei)m_color_attachments.size(), buffers);
		}
		else if (m_color_attachments.empty()) {
			/// Only depth pass
			glDrawBuffer(GL_NONE);
		}

		int check = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		IARA_CORE_ERROR("Framebuffer Status: {0} == {1}", check, GL_FRAMEBUFFER_COMPLETE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_specs.width, m_specs.height);
	}

	void OpenGLFrameBuffer::unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::resize(uint32_t w, uint32_t h) {

		/// This never happens but I'll keep it here
		if (w == 0 || h == 0 || w > s_maxFramebufferSize || h > s_maxFramebufferSize) {
			IARA_CORE_WARN("Attempted to resize the framebuffer to: {0} x {1}", w, h);
			return;
		}

		m_specs.width = w;
		m_specs.height = h;

		invalidate();
	}
	
	int OpenGLFrameBuffer::readPixel(uint32_t att_index, int x, int y) {
		glReadBuffer(GL_COLOR_ATTACHMENT0 + att_index);
		int pixel_data;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixel_data);

		return pixel_data;
	}

	void OpenGLFrameBuffer::clearAttachment(uint32_t att_indx, int value) {
		//IARA_ASSERT(att_indx < m_color_attachments.size());
		
		auto& spec = m_color_att_specs[att_indx];

		glClearTexImage(m_color_attachments[att_indx], 0,
			utils::iaraFramebufferTexFormatToGL(spec.texture_format), GL_INT, &value);

	}
}

