#pragma once

#include "Buffer.h"
#include <memory>

namespace iara {

	class VertexArray {
	public:
		virtual ~VertexArray() {}

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

		virtual const Ref<IndexBuffer>& getIndexBuffer() const = 0;
		virtual const std::vector<Ref<VertexBuffer>>& getVertexBuffers() const = 0;

		static Ref<VertexArray> Create();
	};

}