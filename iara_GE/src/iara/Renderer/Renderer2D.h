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

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>

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

		static void BeginScene(const Camera& camera, const glm::mat4& transform, const uint32_t& plights, const uint32_t& dlight);
		static void BeginScene(EditorCamera& camera, const uint32_t& plights, const uint32_t& dlight);
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
		static void drawLight(const glm::mat4& transform, const PointLightComponent& light, Camera& camera, int entityID);
		static void drawDirLight(const DirLightComponent& dlight);


		/// TEMPORARY
		static void drawShadowMapToQuad(uint32_t shadowmap);
		
		static void ResetStats();
		static Statistics getStats();
	};
}


namespace iara {
	class Renderer3D {
	public:
		static void Renderer3D::Init3D();
		/// SKYBOX
		static void drawSkyBox(const glm::mat4& view_proj, const Ref<Texture2D>& skybox);

	};
}

namespace iara {

	class MeshRenderer {
	public:
		static void InitMeshRenderer();
		static void Shutdown();

		static void BeginShadowMapPass(const glm::mat4& transform);
		static void BeginSceneMesh(const Camera& camera, const glm::mat4& transform, const glm::mat4& light_vp);
		static void BeginSceneMesh(EditorCamera& camera, const glm::mat4& light_vp);
		
		static void EndShadowMapPass();
		static void EndSceneMesh(uint32_t shadowmap);

		/// This will load the mesh, and store the data inside the VAO, VBO and so on, preparing data for flush at the end of the scene
		static void drawMesh(const glm::mat4& transform, MeshComponent& mesh /*OR STRING PATH*/, int entityID);
	private:
		static void FlushMesh(uint32_t shadowmap);
		static void FlushMeshShadowMapPass();
	};
}