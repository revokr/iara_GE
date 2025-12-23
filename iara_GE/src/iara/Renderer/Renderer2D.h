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
//#include <glad\glad.h>

#define GL_RGBA32F 0x8814
#define GL_RGB32F 0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F 0x881B
#define GL_RGBA 0x1908
#define GL_RGB 0x1907
#define GL_FLOAT 0x1406

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
		static void drawLight(const glm::mat4& transform, const PointLightComponent& light, EditorCamera& camera, int entityID);
		static void drawLight(const glm::mat4& transform, const PointLightComponent& light, Camera& camera, int entityID);
		static void drawDirLight(const DirLightComponent& dlight);

		static void renderLumen(uint32_t tex2198, uint32_t tex2199, uint32_t tex2200);
		static void drawAtmosphere(uint32_t vp_width, uint32_t vp_height, const glm::mat4& model_from_view, const glm::mat4& view_from_clip, const glm::vec3& camera_pos, const glm::vec3& white_point, const glm::vec3& earth_center,
								const glm::vec3& sun_dir, const glm::vec2& sun_size, float exposure, uint32_t transmittance_tex, uint32_t scattering_tex,
								uint32_t mie_scattering_tex, uint32_t irradiance_tex, uint32_t lighting_pass_tex, uint32_t g_depth, uint32_t g_position);

		/// TEMPORARY
		static void drawShadowMapToQuad(uint32_t shadowmap);
		static void applyToneMapping(uint32_t hdr_texture, float exposure);
		static void drawFullScreenQuad(const std::vector<bool>& enable_blend);

		static void ResetStats();
		static Statistics getStats();
	};
}


namespace iara {
	class Renderer3D {
	public:
		static void Renderer3D::Init3D();
		/// SKYBOX
		static void drawSkyBox(const glm::mat4& view_proj, const Ref<Texture2D>& skybox, const glm::vec4& sun_direction);

		static void drawDynamicSky(const glm::vec2& resolution, const glm::vec2& mouse_pos, float time);
	};
}

namespace iara {

	class MeshRenderer {
	public:
		static void InitMeshRenderer();
		static void Shutdown();

		static Ref<ShaderLibrary> getShaderLibrary();

		static void ShadowMapPass(const glm::mat4& transform);
		static void ForwardPass(const Camera& camera, const glm::mat4& transform, const glm::mat4& light_vp, uint32_t shadowmap);
		static void ForwardPass(EditorCamera& camera, const glm::mat4& light_vp, uint32_t shadowmap);

		static void GeometryPassGBuffer(EditorCamera& camera);
		static void GeometryPassGBuffer(const Camera& camera, const glm::mat4& transform);

		static void SSAOPass(EditorCamera& camera, uint32_t vp_width, uint32_t vp_height, uint32_t gposition, uint32_t gnormal, uint32_t entityID_map);
		static void SSAOPass(const Camera& camera, const glm::mat4& transform, uint32_t vp_width, uint32_t vp_height, uint32_t gposition, uint32_t gnormal, uint32_t entityID_map);
		static void SSAOBlurPass(uint32_t ssao_input);

		static void LighintgPass(EditorCamera& camera, uint32_t gposition, uint32_t gnormal, uint32_t gdiffusespec, uint32_t entityID_map, uint32_t shadowmap, uint32_t ssao_map, const glm::mat4& light_vp, bool ssao_state);
		static void LighintgPass(const Camera& camera, const glm::mat4& transform, uint32_t gposition, uint32_t gnormal, uint32_t gdiffusespec, uint32_t entityID_map, uint32_t shadowmap, uint32_t ssao_map, const glm::mat4& light_vp, bool ssao_state);

		/// This will load the mesh, and store the data inside the VAO, VBO and so on, preparing data for flush at the end of the scene
		static void drawMesh(const glm::mat4& transform, MeshComponent& mesh /*OR STRING PATH*/, int entityID);

		static void removeMesh(const std::string& path);
	private:
		static void FlushMesh(uint32_t shadowmap);
		static void FlushMeshGeometryPass();
		static void FlushMeshGeometryPassShadowMap();
	};

	
}