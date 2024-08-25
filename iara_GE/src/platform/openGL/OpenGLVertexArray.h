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

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

		virtual const std::shared_ptr<IndexBuffer>& getIndexBuffer() const override;
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const override;

	private:
		uint32_t m_RendererID;

		std::vector<std::shared_ptr<VertexBuffer>> m_vertexbuffers;
		std::shared_ptr<IndexBuffer> m_indexbuffer;
	};

}