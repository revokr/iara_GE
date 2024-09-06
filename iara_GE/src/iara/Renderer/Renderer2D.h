#pragma once

#include "Camera.h"
#include "iara/Renderer/Texture.h"

namespace iara {

	class Renderer2D {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		// Primitives
		static void drawQuadC(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
		static void drawQuadC(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color);
		static void drawQuadRC(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void drawQuadRC(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
	

		static void drawQuadT(const glm::vec2& pos, const glm::vec2& size, const Ref<Texture2D>& texture, float tiling_mult = 1.0f);
		static void drawQuadT(const glm::vec3& pos, const glm::vec2& size, const Ref<Texture2D>& texture, float tiling_mult = 1.0f);
		static void drawQuadTC(const glm::vec2& pos, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& color, float tiling_mult = 1.0f);
		static void drawQuadTC(const glm::vec3& pos, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& color, float tiling_mult = 1.0f);
		static void drawQuadRT(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tiling_m = 1.0f);
		static void drawQuadRT(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tiling_m = 1.0f);
	};

}