#pragma once

#include "Camera.h"
#include "iara/Renderer/Texture.h"
#include "EditorCamera.h"
#include "iara\Scene\Component.h"

//namespace iara {
//	class Renderer3D {
//	public:
//		static void Init3D();
//		static void Shutdown3D();
//
//		static void BeginScene3D(const Camera& camera, const glm::mat4& transform);
//		static void BeginScene3D(EditorCamera& camera);
//		static void BeginScene3D(const OrthographicCamera& camera);
//		static void EndScene3D();
//		static void Flush3D();
//		static void Reset3D();
//
//		// Primitives
//		static void drawCubeC(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
//		static void drawCubeC(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color);
//		static void drawCubeRC(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
//		static void drawCubeRC(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
//		static void drawCubeC(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
//
//
//		struct Statistics {
//			uint32_t draw_calls_3d = 0;
//			uint32_t cube_count = 0;
//
//			uint32_t GetVertices() { return cube_count * 4; }
//			uint32_t GetIndices() { return cube_count * 6; }
//		};
//		static void ResetStats3D();
//		static Statistics getStats3D();
//	};
//}