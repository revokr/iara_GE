#pragma once

#include "iara/Core/core.h"
#include "glm/glm.hpp"

namespace iara {

	enum class FramebufferTextureFormat {
		None = 0,

		/// Color
		RGBA8,
		RED_INTEGER,

		/// Depth/Stencil
		DEPTH24STENCIL8,

		/// Depth for shadowMap
		DEPTH_COMPONENT,

		/// Defaults
		Depth = DEPTH24STENCIL8
	};

	struct FramebufferTextureSpecification {
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: texture_format{ format } {}

		FramebufferTextureFormat texture_format = FramebufferTextureFormat::None;
	};

	struct FramebufferAttachmentSpecification {
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> att) :
			attachments{ att } {}

		std::vector<FramebufferTextureSpecification> attachments;
	};

	struct FramebufferSpecification {
		uint32_t width, height;
		FramebufferAttachmentSpecification attachments;
		uint32_t samples = 1;

		bool swap_chain_target = false;
	};

	class Framebuffer {
	public:
		virtual ~Framebuffer() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;
		virtual void resize(uint32_t w, uint32_t h) = 0;

		virtual int readPixel(uint32_t att_index, int x, int y) = 0;
		virtual void clearAttachment(uint32_t att_indx, int value) = 0;

		virtual uint32_t getColorAtt(uint32_t index = 0) const = 0;
		virtual uint32_t getDepthAtt() const = 0;
		virtual uint32_t getRendererID() const = 0;

		/// virtual FramebufferSpecification getSpecification() = 0;
		virtual const FramebufferSpecification& getSpecification() const = 0;

		static Ref<Framebuffer> CreateMSAA(const FramebufferSpecification& frame_buf_specs);
		static Ref<Framebuffer> Create(const FramebufferSpecification& frame_buf_specs);
		static Ref<Framebuffer> Create(const FramebufferSpecification& frame_buf_specs, const std::string& name);
	};

}