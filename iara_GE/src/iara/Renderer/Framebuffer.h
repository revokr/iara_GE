#pragma once

#include "iara/Core/core.h"
#include "glm/glm.hpp"

namespace iara {

	struct FramebufferSpecification {
		uint32_t width, height;
		uint32_t samples = 1;

		bool swap_chain_target = false;
	};

	class Framebuffer {
	public:
		virtual ~Framebuffer() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;
		virtual void resize(uint32_t w, uint32_t h) = 0;

		virtual uint32_t getColorAtt() const = 0;
		virtual uint32_t getDepthAtt() const = 0;
		virtual uint32_t getRendererID() const = 0;

		/// virtual FramebufferSpecification getSpecification() = 0;
		virtual const FramebufferSpecification& getSpecification() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& frame_buf_specs);
	};

}