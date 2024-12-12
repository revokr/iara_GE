#pragma once

#include "Camera.h"
#include "iara/Renderer/Texture.h"
#include "EditorCamera.h"
#include "iara\Scene\Component.h"

#include "VertexArray.h"
#include "shader.h"
#include "platform/openGL/OpenGLShader.h"
#include "iara\Renderer\UniformBuffer.h"
#include "RenderCommand.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

namespace iara {

	struct Statistics {
		uint32_t draw_calls = 0;
		uint32_t quad_count = 0;
		uint32_t draw_calls_3d = 0;
		uint32_t cube_count = 0;

		uint32_t GetVertices() { return quad_count * 4; }
		uint32_t GetIndices() { return quad_count * 6; }

		uint32_t GetVertices3D() { return cube_count * 36; }
		uint32_t GetIndices3D() { return cube_count * 36; }
	};


	class Renderer2D {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform, const uint32_t& plights);
		static void BeginScene(EditorCamera& camera, const uint32_t& plights);
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();
		static void Flush();
		static void Reset();

		// Primitives
		static void drawQuadC(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
		static void drawQuadC(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color);
		static void drawQuadRC(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void drawQuadRC(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
	
		static void drawQuadC(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		static void drawQuadT(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& color, float tiling_mult = 1.0f, int entityID = -1);
		static void drawQuadTBillboard(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& color, EditorCamera& camera, float tiling_mult = 1.0f, int entityID = -1);
		static void drawQuadRC(const glm::mat4& transform, float rotation, const glm::vec4& color);

		static void drawQuadT(const glm::vec2& pos, const glm::vec2& size, const Ref<Texture2D>& texture, float tiling_mult = 1.0f);
		static void drawQuadT(const glm::vec3& pos, const glm::vec2& size, const Ref<Texture2D>& texture, float tiling_mult = 1.0f);
		static void drawQuadTC(const glm::vec2& pos, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& color, float tiling_mult = 1.0f);
		static void drawQuadTC(const glm::vec3& pos, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& color, float tiling_mult = 1.0f);
		static void drawQuadRT(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tiling_m = 1.0f);
		static void drawQuadRT(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tiling_m = 1.0f);
	
		static void drawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);
		static void drawLight(const glm::mat4& transform ,const PointLightComponent& light, EditorCamera& camera, int entityID);
		static void drawDirLight(const DirLightComponent& dlight);
		
		static void ResetStats();
		static Statistics getStats();
	};
}


namespace iara {
	class Renderer3D {
	public:
		static void Init3D();
		static void Shutdown3D();

		static void BeginScene3D(const Camera& camera, const glm::mat4& transform);
		static void BeginScene3D(EditorCamera& camera);
		static void EndScene3D();
		static void Flush3D();
		static void Reset3D();

		// Primitives
		static void drawCubeC(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		static void drawCubeCT(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& color, int entityID = -1);
		static void drawCubeM(const glm::mat4& transform, const glm::vec4& color, int material_index, int entityID = -1);

		//static void drawLightSource(const glm::vec3& pos, const glm::vec3& size, int entityID = -1);
		static void addMaterial(const glm::vec4& ambient, const glm::vec4& diffuse, const glm::vec4& specular, const float& shininess);
		static void addMaterial(const uint32_t index, const glm::vec4& ambient, const glm::vec4& diffuse, const glm::vec4& specular, const float& shininess);
		
		/// SKYBOX
		static void drawSkyBox(const glm::mat4& view, const glm::mat4& projection);

		static void ResetStats3D();
		static Statistics getStats3D();
		static uint32_t getNrMaterials();
		static Material getMaterial(uint32_t index);
	};
}