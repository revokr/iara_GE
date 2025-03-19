#pragma once

#include "iara/Renderer/RendererAPI.h"

namespace iara {

	class OpenGLRendererAPI : public RendererAPI {
	public:
		virtual void Init() override;
		virtual void setViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void setDepthMask(bool set) override;

		virtual void DrawIndexed(const Ref<VertexArray>& va, uint32_t indexCount = 0) override;
		virtual void DrawIndexedBaseVertex(const Ref<VertexArray>& va, uint32_t indexCount, uint32_t indexStart, uint32_t vertexStart) override;
		virtual void drawArray(const Ref<VertexArray>& va, uint32_t start, uint32_t end) override;
	};

} 