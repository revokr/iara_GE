#pragma once

#include "glm/glm.hpp"
#include "VertexArray.h"

namespace iara {

	class RendererAPI {
	public:
		enum class API {
			None = 0, OpenGL = 1
		};

	public:
		virtual void Init() = 0;
		virtual void setViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void setDepthMask(bool set) = 0;
		virtual void polygonMode(bool enable) = 0;
		virtual void BindTextureUnit(uint32_t slot, uint32_t tex) = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& va, uint32_t indexCount = 0) = 0;
		virtual void drawArray(const Ref<VertexArray>& va, uint32_t start, uint32_t end) = 0;
		virtual void DrawIndexedBaseVertex(const Ref<VertexArray>& va, uint32_t indexCount, uint32_t indexStart, uint32_t vertexStart) = 0;

		inline static API getAPI() { return s_API; }
	private:
		static API s_API;
	};
}