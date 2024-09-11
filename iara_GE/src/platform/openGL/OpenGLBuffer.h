#pragma once

#include "iara/Renderer/Buffer.h"

namespace iara {

	class OpenGLVertexBuffer : public VertexBuffer {
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(float* vertices, uint32_t size);

		virtual ~OpenGLVertexBuffer();

		virtual void bind() const override;
		virtual void unbind() const override;

		virtual void SetData(const void* data, uint32_t size) override;

		virtual void setLayout(const BufferLayout& layout) override { m_layout = layout; }
		virtual const BufferLayout& getLayout() const override { return m_layout; }

	private:
		uint32_t m_RendererID;
		BufferLayout m_layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer {
	public:
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);

		virtual ~OpenGLIndexBuffer();

		virtual void bind() const;
		virtual void unbind() const;
		virtual uint32_t getCount() const { return m_count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_count;
	};

}