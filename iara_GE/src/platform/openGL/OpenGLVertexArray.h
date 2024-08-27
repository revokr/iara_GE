#pragma once

#include "iara/Renderer/VertexArray.h"
#include <vector>

namespace iara {

	class OpenGLVertexArray : public VertexArray {
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void bind() const override;
		virtual void unbind() const override;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

		virtual const Ref<IndexBuffer>& getIndexBuffer() const override;
		virtual const std::vector<Ref<VertexBuffer>>& getVertexBuffers() const override;

	private:
		uint32_t m_RendererID;

		std::vector<Ref<VertexBuffer>> m_vertexbuffers;
		Ref<IndexBuffer> m_indexbuffer;
	};

}