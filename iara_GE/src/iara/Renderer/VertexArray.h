#pragma once

#include "Buffer.h"
#include <memory>

namespace iara {

	class VertexArray {
	public:
		virtual ~VertexArray() {}

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

		virtual const std::shared_ptr<IndexBuffer>& getIndexBuffer() const = 0;
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const = 0;

		static VertexArray* Create();
	};

}