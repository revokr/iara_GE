#pragma once

#include "RendererAPI.h"

namespace iara {

	class RenderCommand {
	public:
		inline static void Init() { s_RendererAPI->Init(); }

		inline static void setViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) { s_RendererAPI->setViewPort(x, y, width, height); }

		inline static void SetClearColor(const glm::vec4& color) {
			s_RendererAPI->SetClearColor(color);
		}

		inline static void ClearColorBuffer() {
			s_RendererAPI->ClearColorBuffer();
		}

		inline static void Clear() {
			s_RendererAPI->Clear();
		}

		inline static void setDepthMask(bool set) {
			s_RendererAPI->setDepthMask(set);
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) {
			s_RendererAPI->DrawIndexed(vertexArray, indexCount); }

		inline static void drawArrays(const Ref<VertexArray>& vertexArray, uint32_t start, uint32_t end) {
			s_RendererAPI->drawArray(vertexArray, start, end);
		}

		inline static void DrawIndexedBaseVertex(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t indexStart, uint32_t vertexStart) {
			s_RendererAPI->DrawIndexedBaseVertex(vertexArray, indexCount, indexStart, vertexStart);
		}

		inline static void polygonMode(bool enable) {
			s_RendererAPI->polygonMode(enable);
		}

		inline static void BindTextureUnit(uint32_t slot, uint32_t tex) {
			s_RendererAPI->BindTextureUnit(slot, tex);
		}

		inline static void ActiveBindTexture2D(uint32_t slot, uint32_t tex) {
			s_RendererAPI->ActiveBindTexture2D(slot, tex);
		}

		inline static void ActiveBindTexture3D(uint32_t slot, uint32_t tex) {
			s_RendererAPI->ActiveBindTexture3D(slot, tex);
		}

		inline static void BlendEnablei(uint32_t i) {
			s_RendererAPI->BlendEnablei(i);
		}

		inline static void BlendDisablei(uint32_t i) {
			s_RendererAPI->BlendDisablei(i);
		}

		inline static void drawArraysStrip(uint32_t start, uint32_t end) {
			s_RendererAPI->drawArrayStrip(start, end);
		}
	private:
		static RendererAPI* s_RendererAPI;
	};

}