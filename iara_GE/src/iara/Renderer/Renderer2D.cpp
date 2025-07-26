#include "ir_pch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "shader.h"
#include "platform/openGL/OpenGLShader.h"
#include "iara\Renderer\UniformBuffer.h"
#include "RenderCommand.h"
#include "iara\Math\Math.h"

#include <thread>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <glm/ext/matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#define rad(x) glm::radians(x)
#define MAX_MATERIALS 10
#define MAX_LIGHTS 10

namespace iara {

	struct QuadVertex {
		glm::vec3 position;
		glm::vec4 color;
		glm::vec2 tex_coord;
		float tex_index;
		float tiling_mult;

		/// Editor Only
		int entityID;
	};

	struct CubeVertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec4 color;
		glm::vec2 tex_coord;
		float tex_index;
		float tiling_mult;

		/// Editor Only
		int entityID;
	};

	struct Renderer_Storeage {

		uint32_t FRAMES = 0;

		const uint32_t MaxQuads = 100;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTexSlots = 32;

		uint32_t current_lights;

		PointLight point_lights[MAX_LIGHTS];
		uint32_t scene_plights;
		DirLight skyLight;

		Ref<VertexArray> vao;
		Ref<VertexBuffer> vertexBuffer;
		Ref<Shader> tex_shader;
		Ref<Texture2D> white_tex;

		uint32_t QuadIndCnt = 0;
		QuadVertex* quadVertexBufferBase = nullptr;
		QuadVertex* quadVertexBufferPtr = nullptr;


		std::array<Ref<Texture2D>, MaxTexSlots> texture_slots;
		uint32_t textureSlotInd = 1; /// 0 = white texture

		glm::vec4 quadVertices[4];
		glm::vec2 texCoords[4];

		Statistics stats;

		struct CameraData {
			glm::mat4 view_projection3D;
			glm::vec4 camPos;
		};

		struct PointLightsData {
			PointLight point_lights[MAX_LIGHTS];
			int nrLights;
		};

		struct DirLightData {
			DirLight dlight;
		};

		CameraData camera_buffer;
		Ref<UniformBuffer> camera_uniform_buffer;
	};

	static Renderer_Storeage s_Data;

	struct ShadowMapData {
		Ref<VertexArray> shadow_quad_vao;
		Ref<VertexBuffer> shadow_quad_vb;
		Ref<IndexBuffer> shadow_quad_ib;

		Ref<Shader> quad_shadowmap_shader;
	};

	static ShadowMapData s_shadowMapData;

	void Renderer2D::Init() {
		IARA_PROFILE_FUNCTION();

		/// SHADOWMAP

		s_shadowMapData.shadow_quad_vao = VertexArray::Create();
		float quadVertices[] = {
			//  positions        // texCoords
			-1.0f,  1.0f, 0.0f,   0.0f, 1.0f, // top-left
			-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f, 0.0f,   1.0f, 0.0f, // bottom-right

			-1.0f,  1.0f, 0.0f,   0.0f, 1.0f, // top-left
			 1.0f, -1.0f, 0.0f,   1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, 0.0f,   1.0f, 1.0f  // top-right
		};
		s_shadowMapData.quad_shadowmap_shader = Shader::Create("shadowMapQuad", "Shaders/shadowmap_quad.vert", "Shaders/shadowmap_quad.frag");
		s_shadowMapData.shadow_quad_vb = VertexBuffer::Create(quadVertices, sizeof(float) * 30);

		s_shadowMapData.shadow_quad_vb->setLayout({
			{ ShaderDataType::Float3, "a_pos" },
			{ ShaderDataType::Float2, "a_tex_coord" }
			});
		s_shadowMapData.shadow_quad_vao->AddVertexBuffer(s_shadowMapData.shadow_quad_vb);

		///-------------

		s_Data.vao = (VertexArray::Create());

		s_Data.vertexBuffer = (VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex)));

		s_Data.vertexBuffer->setLayout({
			{ ShaderDataType::Float3, "a_pos" },
			{ ShaderDataType::Float4, "a_color" },
			{ ShaderDataType::Float2, "a_tex" },
			{ ShaderDataType::Float,  "a_tex_id" },
			{ ShaderDataType::Float,  "a_tiling_mult" },
			{ ShaderDataType::Int,	  "a_entityID"}
		});
		s_Data.vao->AddVertexBuffer(s_Data.vertexBuffer);

		s_Data.quadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6) {
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.vao->SetIndexBuffer(indexBuffer);
		delete[] quadIndices;

		s_Data.white_tex = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.white_tex->setData(&whiteTextureData, sizeof(uint32_t));

		int samplers[s_Data.MaxTexSlots];
		for (uint32_t i = 0; i < s_Data.MaxTexSlots; i++) {
			samplers[i] = i;
		}

		
		s_Data.tex_shader = Shader::Create("texture", "Shaders/texture.vert", "Shaders/texture.frag");
		s_Data.tex_shader->bind();
		s_Data.tex_shader->setUniformIntArray("u_textures", samplers, s_Data.MaxTexSlots);

		s_Data.texture_slots[0] = s_Data.white_tex;

		s_Data.quadVertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.quadVertices[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.quadVertices[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.quadVertices[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		s_Data.texCoords[0] = { 0.0f, 0.0f };
		s_Data.texCoords[1] = { 1.0f, 0.0f };
		s_Data.texCoords[2] = { 1.0f, 1.0f };
		s_Data.texCoords[3] = { 0.0f, 1.0f };

		s_Data.camera_uniform_buffer = UniformBuffer::Create(sizeof(Renderer_Storeage::CameraData), 0);

	}

	void Renderer2D::Shutdown() {
		delete[] s_Data.quadVertexBufferBase;
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform, const uint32_t& plights, const uint32_t& dlight) {
		glm::mat4 viewproj = camera.getProjection() * glm::inverse(transform);

		s_Data.camera_buffer.view_projection3D = camera.getProjection() * glm::inverse(transform);
		s_Data.camera_buffer.camPos = glm::vec4(transform[3][0], transform[3][1], transform[3][2], 1.0f);
		s_Data.camera_uniform_buffer->setData(&s_Data.camera_buffer, sizeof(Renderer_Storeage::CameraData));
		s_Data.QuadIndCnt = 0;
		s_Data.current_lights = 0;
		s_Data.quadVertexBufferPtr = s_Data.quadVertexBufferBase;

		s_Data.scene_plights = plights;

		s_Data.textureSlotInd = 1;
	}

	void Renderer2D::BeginScene(EditorCamera& camera, const uint32_t& plights, const uint32_t& dlight) {
		s_Data.tex_shader->bind();

		s_Data.camera_buffer.view_projection3D = camera.getViewProjection();
		auto camPos = camera.getPosition();
		s_Data.camera_buffer.camPos = glm::vec4(camPos.x, camPos.y, camPos.z, 1.0f);
		s_Data.camera_uniform_buffer->setData(&s_Data.camera_buffer, sizeof(Renderer_Storeage::CameraData));
		s_Data.QuadIndCnt = 0;
		s_Data.current_lights = 0;
		s_Data.quadVertexBufferPtr = s_Data.quadVertexBufferBase;

		s_Data.scene_plights = plights;

		s_Data.textureSlotInd = 1;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera) {
		s_Data.tex_shader->bind();

		s_Data.camera_buffer.view_projection3D = camera.getVP();
		s_Data.camera_uniform_buffer->setData(&s_Data.camera_buffer, sizeof(Renderer_Storeage::CameraData));
		s_Data.QuadIndCnt = 0;
		s_Data.quadVertexBufferPtr = s_Data.quadVertexBufferBase;

		s_Data.textureSlotInd = 1;
	}

	void Renderer2D::EndScene() {
		for (uint32_t i = 0; i < s_Data.textureSlotInd; i++) {
			s_Data.texture_slots[i]->bind(i);
		}

		Flush();
	}

	void Renderer2D::Flush() {

		uint32_t data_size = (uint32_t)((uint8_t*)s_Data.quadVertexBufferPtr - (uint8_t*)s_Data.quadVertexBufferBase);
		s_Data.vertexBuffer->SetData(s_Data.quadVertexBufferBase, data_size);

		s_Data.tex_shader->bind();
		s_Data.vao->bind();
		RenderCommand::DrawIndexed(s_Data.vao, s_Data.QuadIndCnt);

		s_Data.stats.draw_calls++;
	}

	void Renderer2D::Reset() {
		s_Data.QuadIndCnt = 0;
		s_Data.quadVertexBufferPtr = s_Data.quadVertexBufferBase;

		s_Data.textureSlotInd = 1;
	}

	void Renderer2D::drawQuadC(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
		drawQuadC({ pos.x, pos.y, 0.0f }, size, color);
	}

	void Renderer2D::drawQuadC(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color) {

		if (s_Data.QuadIndCnt >= s_Data.MaxIndices) {
			EndScene();
			Reset();
		}

		const float textureInd = 0.0f; /// White texture
		const float tiling_mult = 1.0f;

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < 4; i++) {
			s_Data.quadVertexBufferPtr->position = transform * s_Data.quadVertices[i];
			s_Data.quadVertexBufferPtr->color = color;
			s_Data.quadVertexBufferPtr->tex_coord = s_Data.texCoords[i];
			s_Data.quadVertexBufferPtr->tex_index = textureInd;
			s_Data.quadVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.quadVertexBufferPtr++;
		}

		s_Data.QuadIndCnt += 6;

		s_Data.stats.quad_count++;
	}

	void Renderer2D::drawQuadT(const glm::vec2& pos, const glm::vec2& size, const Ref<Texture2D>& texture, float tiling_mult) {
		drawQuadT({ pos.x, pos.y, 0.0f }, size, texture, tiling_mult);
	}

	void Renderer2D::drawQuadT(const glm::vec3& pos, const glm::vec2& size, const Ref<Texture2D>& texture, float tiling_mult) {
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		drawQuadT(transform, texture, { 1.0f, 1.0f, 1.0f, 1.0f }, tiling_mult);
	}

	void Renderer2D::drawQuadTC(const glm::vec2& pos, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& color, float tiling_mult) {
		drawQuadTC({ pos.x, pos.y, 0.0f }, size, texture, color, tiling_mult);
	}

	void Renderer2D::drawQuadTC(const glm::vec3& pos, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& color, float tiling_mult) {

		if (s_Data.QuadIndCnt >= s_Data.MaxIndices) {
			EndScene();
			Reset();
		}

		/// Check if texture exists
		float textureInd = 0.0f;

		for (uint32_t i = 1; i < s_Data.textureSlotInd; i++) {
			if (*s_Data.texture_slots[i].get() == *texture.get()) {
				textureInd = float(i);
				break;
			}
		}

		if (textureInd == 0.0f) {
			textureInd = (float)s_Data.textureSlotInd;
			s_Data.texture_slots[s_Data.textureSlotInd] = texture;
			s_Data.textureSlotInd++;
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < 4; i++) {
			s_Data.quadVertexBufferPtr->position = transform * s_Data.quadVertices[i];
			s_Data.quadVertexBufferPtr->color = color;
			s_Data.quadVertexBufferPtr->tex_coord = s_Data.texCoords[i];
			s_Data.quadVertexBufferPtr->tex_index = textureInd;
			s_Data.quadVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.quadVertexBufferPtr++;
		}

		s_Data.QuadIndCnt += 6;

		s_Data.stats.quad_count++;
	}

	void Renderer2D::drawQuadRC(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color) {
		drawQuadRC({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::drawQuadRC(const glm::vec3& pos, const glm::vec2& size, float rotation, const glm::vec4& color) {

		if (s_Data.QuadIndCnt >= s_Data.MaxIndices) {
			EndScene();
			Reset();
		}

		const float textureInd = 0.0f; /// White texture
		const float tiling_mult = 1.0f;

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < 4; i++) {
			s_Data.quadVertexBufferPtr->position = transform * s_Data.quadVertices[i];
			s_Data.quadVertexBufferPtr->color = color;
			s_Data.quadVertexBufferPtr->tex_coord = s_Data.texCoords[i];
			s_Data.quadVertexBufferPtr->tex_index = textureInd;
			s_Data.quadVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.quadVertexBufferPtr++;
		}


		s_Data.QuadIndCnt += 6;

		s_Data.stats.quad_count++;
	}

	void Renderer2D::drawQuadC(const glm::mat4& transform, const glm::vec4& color, int entityID) {
		if (s_Data.QuadIndCnt >= s_Data.MaxIndices) {
			EndScene();
			Reset();
		}

		const float textureInd = 0.0f; /// White texture
		const float tiling_mult = 1.0f;

		for (size_t i = 0; i < 4; i++) {
			s_Data.quadVertexBufferPtr->position = transform * s_Data.quadVertices[i];
			s_Data.quadVertexBufferPtr->color = color;
			s_Data.quadVertexBufferPtr->tex_coord = s_Data.texCoords[i];
			s_Data.quadVertexBufferPtr->tex_index = textureInd;
			s_Data.quadVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.quadVertexBufferPtr->entityID = entityID;
			s_Data.quadVertexBufferPtr++;
		}

		s_Data.QuadIndCnt += 6;

		s_Data.stats.quad_count++;
	}

	void Renderer2D::drawQuadT(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& color, float tiling_mult, int entityID) {
		if (s_Data.QuadIndCnt >= s_Data.MaxIndices) {
			EndScene();
			Reset();
		}

		/// Check if texture exists
		float textureInd = 0.0f;

		for (uint32_t i = 1; i < s_Data.textureSlotInd; i++) {
			if (*s_Data.texture_slots[i].get() == *texture.get()) {
				textureInd = float(i);
				break;
			}
		}

		if (textureInd == 0.0f) {
			textureInd = (float)s_Data.textureSlotInd;
			s_Data.texture_slots[s_Data.textureSlotInd] = texture;
			s_Data.textureSlotInd++;
		}

		for (size_t i = 0; i < 4; i++) {
			s_Data.quadVertexBufferPtr->position = transform * s_Data.quadVertices[i];
			s_Data.quadVertexBufferPtr->color = color;
			s_Data.quadVertexBufferPtr->tex_coord = s_Data.texCoords[i];
			s_Data.quadVertexBufferPtr->tex_index = textureInd;
			s_Data.quadVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.quadVertexBufferPtr->entityID = entityID;
			s_Data.quadVertexBufferPtr++;
		}

		s_Data.QuadIndCnt += 6;

		s_Data.stats.quad_count++;
	}

	void Renderer2D::drawQuadTBillboard(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& color, EditorCamera& camera,  float tiling_mult,int entityID) {
		if (s_Data.QuadIndCnt >= s_Data.MaxIndices) {
			EndScene();
			Reset();
		}

		/// Check if texture exists
		float textureInd = 0.0f;

		for (uint32_t i = 1; i < s_Data.textureSlotInd; i++) {
			if (*s_Data.texture_slots[i].get() == *texture.get()) {
				textureInd = float(i);
				break;
			}
		}

		if (textureInd == 0.0f) {
			textureInd = (float)s_Data.textureSlotInd;
			s_Data.texture_slots[s_Data.textureSlotInd] = texture;
			s_Data.textureSlotInd++;
		}


		glm::mat4 viewMat = camera.getViewMatrix();
		glm::vec3 camera_right = { viewMat[0][0], viewMat[1][0], viewMat[2][0] };
		glm::vec3 camera_up = { viewMat[0][1], viewMat[1][1], viewMat[2][1] };
		glm::vec3 trans, rot, scal;
		math::decomposeTransform(transform, trans, rot, scal);
		
		for (size_t i = 0; i < 4; i++) {
			s_Data.quadVertexBufferPtr->position = trans + (camera_right * s_Data.quadVertices[i].x * scal.x + camera_up * s_Data.quadVertices[i].y * scal.y);
			s_Data.quadVertexBufferPtr->color = color;
			s_Data.quadVertexBufferPtr->tex_coord = s_Data.texCoords[i];
			s_Data.quadVertexBufferPtr->tex_index = textureInd;
			s_Data.quadVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.quadVertexBufferPtr->entityID = entityID;
			s_Data.quadVertexBufferPtr++;
		}

		s_Data.QuadIndCnt += 6;

		s_Data.stats.quad_count++;
	}

	void Renderer2D::drawQuadRC(const glm::mat4& transform, float rotation, const glm::vec4& color) {
		if (s_Data.QuadIndCnt >= s_Data.MaxIndices) {
			EndScene();
			Reset();
		}

		const float textureInd = 0.0f; /// White texture
		const float tiling_mult = 1.0f;

		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f });
		glm::mat4 transform1 = rotate * transform;

		for (size_t i = 0; i < 4; i++) {
			s_Data.quadVertexBufferPtr->position = transform1 * s_Data.quadVertices[i];
			s_Data.quadVertexBufferPtr->color = color;
			s_Data.quadVertexBufferPtr->tex_coord = s_Data.texCoords[i];
			s_Data.quadVertexBufferPtr->tex_index = textureInd;
			s_Data.quadVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.quadVertexBufferPtr++;
		}

		s_Data.QuadIndCnt += 6;

		s_Data.stats.quad_count++;
	}

	void Renderer2D::drawQuadRT(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tiling_m) {
		drawQuadRT({ position.x, position.y, 0.0f }, size, rotation, texture, tiling_m);
	}

	void Renderer2D::drawQuadRT(const glm::vec3& pos, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tiling_mult) {

		if (s_Data.QuadIndCnt >= s_Data.MaxIndices) {
			EndScene();
			Reset();
		}

		const glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		/// Check if texture existsas
		float textureInd = 0.0f;

		for (uint32_t i = 1; i < s_Data.textureSlotInd; i++) {
			if (*s_Data.texture_slots[i].get() == *texture.get()) {
				textureInd = float(i);
				break;
			}
		}

		if (textureInd == 0.0f) {
			textureInd = (float)s_Data.textureSlotInd;
			s_Data.texture_slots[s_Data.textureSlotInd] = texture;
			s_Data.textureSlotInd++;
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f })
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f });

		for (size_t i = 0; i < 4; i++) {
			s_Data.quadVertexBufferPtr->position = transform * s_Data.quadVertices[i];
			s_Data.quadVertexBufferPtr->color = color;
			s_Data.quadVertexBufferPtr->tex_coord = s_Data.texCoords[i];
			s_Data.quadVertexBufferPtr->tex_index = textureInd;
			s_Data.quadVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.quadVertexBufferPtr++;
		}

		s_Data.QuadIndCnt += 6;

		s_Data.stats.quad_count++;
	}

	void Renderer2D::drawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID) {
		if (src.texture) {
			drawQuadT(transform, src.texture, src.color, src.tiling_factor, entityID);
		}
		else {
			drawQuadC(transform, src.color, entityID);
		}
	}

	void Renderer2D::drawLight(const glm::mat4& transform, const PointLightComponent& light, EditorCamera& camera, int entityID) {
		glm::vec4 color = glm::vec4(1.0f);
		drawQuadTBillboard(transform, light.light_tex, color, camera, 1.0f, entityID);

		if (s_Data.current_lights < s_Data.scene_plights) {
			PointLight plight = light.plight;
			plight.position = transform * plight.position;
			s_Data.point_lights[s_Data.current_lights] = plight;
			s_Data.current_lights++;
		}
	}

	void Renderer2D::drawLight(const glm::mat4& transform, const PointLightComponent& light, Camera& camera, int entityID) {
		if (s_Data.current_lights < s_Data.scene_plights) {
			PointLight plight = light.plight;
			plight.position = transform * plight.position;
			s_Data.point_lights[s_Data.current_lights] = plight;
			s_Data.current_lights++;
		}
	}

	void Renderer2D::drawDirLight(const DirLightComponent& dlight) {
		s_Data.skyLight = dlight.dlight;
		//IARA_CORE_ERROR("DE UNDE ESTI AICI????");
	}

	void Renderer2D::drawShadowMapToQuad(uint32_t shadowmap)
	{
		s_shadowMapData.quad_shadowmap_shader->bind();
		s_shadowMapData.shadow_quad_vao->bind();

		RenderCommand::BindTextureUnit(0, shadowmap);
		s_shadowMapData.quad_shadowmap_shader->setUniformInt("depthMap", 0);

		RenderCommand::drawArrays(s_shadowMapData.shadow_quad_vao, 0, 7);

		s_shadowMapData.quad_shadowmap_shader->unbind();

	}

	void Renderer2D::ResetStats() {
		memset(&s_Data.stats, 0, sizeof(Statistics));
	}

	Statistics Renderer2D::getStats() {
		return s_Data.stats;
	}
}

namespace iara {

	struct CubeMap_Resources {
		Ref<Texture2D> m_cubemap;
		Ref<Shader> cubemap_shader;
		Ref<VertexArray> vao_cubemap;
		Ref<VertexBuffer> vb_cubemap;

		struct CameraData_skybox {
			glm::mat4 view_projection3D;
		};

		CameraData_skybox camera_buffer_skybox;
		Ref<UniformBuffer> camera_uniform_buffer_skybox;
	};

	static CubeMap_Resources s_cubemap;

	void Renderer3D::Init3D() {
		s_cubemap.m_cubemap = Texture2D::CreateCubemap("Assets/Textures/skybox2/sky2.png");
		s_cubemap.cubemap_shader = Shader::Create("cubemap", "Shaders/cubemap.vert", "Shaders/cubemap.frag");

		float skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};

		s_cubemap.vao_cubemap = (VertexArray::Create());

		s_cubemap.vb_cubemap = (VertexBuffer::Create(sizeof(skyboxVertices)));
		s_cubemap.vb_cubemap->SetData(skyboxVertices, sizeof(skyboxVertices));

		s_cubemap.vb_cubemap->setLayout({
			{ ShaderDataType::Float3, "a_pos" }
			});
		s_cubemap.vao_cubemap->AddVertexBuffer(s_cubemap.vb_cubemap);

		s_cubemap.camera_uniform_buffer_skybox = UniformBuffer::Create(sizeof(CubeMap_Resources::CameraData_skybox), 20);
	}

	void Renderer3D::drawSkyBox(const glm::mat4& view_proj, const Ref<Texture2D>& skybox) {
		RenderCommand::setDepthMask(false);
		s_cubemap.cubemap_shader->bind();

		skybox->bind();
		s_cubemap.camera_buffer_skybox.view_projection3D = view_proj;
		s_cubemap.camera_uniform_buffer_skybox->setData(&s_cubemap.camera_buffer_skybox, sizeof(CubeMap_Resources::CameraData_skybox));

		s_cubemap.vao_cubemap->bind();
		RenderCommand::drawArrays(s_cubemap.vao_cubemap, 0, 36);
		RenderCommand::setDepthMask(true);
		s_cubemap.cubemap_shader->unbind();
	}

}


namespace iara {

	struct ShaderMaterial {
		glm::vec4 albedo;
		float shininess;
		glm::vec3 padding;
	};

	struct SceneMeshData {
		std::string path;
		std::vector<Material> materials;
		glm::mat4 transform;

		Ref<VertexBuffer> entityID_VB;
	};

	struct ModelLoadTask {
		std::string path;
		int entityID;
		std::promise<void> donePromise;
	};

	static std::queue<ModelLoadTask> g_modelLoadQueue;
	static std::mutex g_modelLoadMutex;
	static std::condition_variable g_modelLoadCV;
	static bool g_terminateLoader = false;
	static std::unordered_map<std::string, std::shared_future<void>> g_pendingLoads;
	static std::thread g_thread;

	struct MeshRendererStoreage {
		Ref<VertexArray> m_vao;
		Ref<Shader> m_shader;

		Ref<Shader> m_shadowmap_shader;

		std::unordered_map<std::string, std::pair<bool, Mesh>> m_stored_meshes_concurrent;
		std::unordered_map<std::string, Mesh> m_stored_meshes;
		std::vector<SceneMeshData> m_scene_meshes;

		struct CameraData {
			glm::mat4 view_projection3D;
			glm::vec4 camPos;
		};

		struct ModelData {
			glm::mat4 model;
		};

		struct MaterialsData {
			ShaderMaterial material;
		};

		struct PointLightsData {
			PointLight point_lights[MAX_LIGHTS];
			int nrLights;
		};

		struct DirLightData {
			DirLight dlight;
		};

		struct LightVPData {
			glm::mat4 light_view_projection;
		};

		CameraData camera_buffer_mesh;
		Ref<UniformBuffer> camera_uniform_buffer_mesh;

		ModelData model_buffer_mesh;
		Ref<UniformBuffer> model_uniform_buffer_mesh;

		DirLightData dlight_buffer_mesh;
		Ref<UniformBuffer> dlight_uniform_buffer_mesh;

		MaterialsData materials_buffer_mesh;
		Ref<UniformBuffer> materials_uniform_buffer_mesh;

		PointLightsData plights_buffer_mesh;
		Ref<UniformBuffer> plights_uniform_buffer_mesh;

		LightVPData light_vp_buffer_shadowmap;
		Ref<UniformBuffer> light_vp_uniform_buffer_shadowmap;

		ModelData model_buffer_shadowmap;
		Ref<UniformBuffer> model_uniform_buffer_shadowmap;
	};

	static MeshRendererStoreage s_MeshData;

	static void modelLoaderThreadFunc() {
		IARA_CORE_TRACE("AM INTRAT IN MESH LOADER!");
		while (!g_terminateLoader) {
			ModelLoadTask task;

			{
				std::unique_lock lock(g_modelLoadMutex);
				g_modelLoadCV.wait(lock, [] { return !g_modelLoadQueue.empty() || g_terminateLoader; });
				IARA_CORE_TRACE("AM TRECUT DE CONDITION VARIABLE");
				if (g_terminateLoader) break;
			}

			task = std::move(g_modelLoadQueue.front());
			g_modelLoadQueue.pop();

			IARA_CORE_TRACE("START MESH LOADING!");

			Mesh mesh;
			mesh.loadModel(task.path, task.entityID);

			{
				std::unique_lock lock(g_modelLoadMutex);
				s_MeshData.m_stored_meshes[task.path] = std::move(mesh);
			}
			IARA_CORE_TRACE("MESH LOADING FINISHED!");

			task.donePromise.set_value();
		}
	}

	void MeshRenderer::InitMeshRenderer() {
		s_MeshData.m_vao = VertexArray::Create();
		s_MeshData.m_shader = Shader::Create("mesh-light", "Shaders/light-mesh.vert", "Shaders/light-mesh.frag");
		s_MeshData.m_shadowmap_shader = Shader::Create("shadowmap", "Shaders/shadowmap.vert", "Shaders/shadowmap.frag");

		s_MeshData.camera_uniform_buffer_mesh        = UniformBuffer::Create(sizeof(MeshRendererStoreage::CameraData), 6);
		s_MeshData.model_uniform_buffer_mesh         = UniformBuffer::Create(sizeof(MeshRendererStoreage::ModelData), 7);
		s_MeshData.materials_uniform_buffer_mesh     = UniformBuffer::Create(sizeof(MeshRendererStoreage::MaterialsData), 8);

		s_MeshData.plights_uniform_buffer_mesh       = UniformBuffer::Create(sizeof(Renderer_Storeage::PointLightsData), 9);
		s_MeshData.dlight_uniform_buffer_mesh        = UniformBuffer::Create(sizeof(Renderer_Storeage::DirLightData), 10);

		s_MeshData.light_vp_uniform_buffer_shadowmap = UniformBuffer::Create(sizeof(MeshRendererStoreage::LightVPData), 11);
		s_MeshData.model_uniform_buffer_shadowmap    = UniformBuffer::Create(sizeof(MeshRendererStoreage::ModelData), 12);

		g_thread = std::thread(modelLoaderThreadFunc);
	}

	void MeshRenderer::Shutdown() {
		{
			std::lock_guard<std::mutex> lock(g_modelLoadMutex);
			g_terminateLoader = true;
		}
		g_modelLoadCV.notify_one();
		if (g_thread.joinable())
			g_thread.join();
	}

	void MeshRenderer::BeginShadowMapPass(const glm::mat4& transform) {
		s_MeshData.m_shadowmap_shader->bind();

		MeshRendererStoreage::LightVPData data;
		data.light_view_projection = transform;

		s_MeshData.light_vp_uniform_buffer_shadowmap->setData(&data, sizeof(MeshRendererStoreage::LightVPData));
	}

	void MeshRenderer::BeginSceneMesh(const Camera& camera, const glm::mat4& transform, const glm::mat4& light_vp) {
		s_MeshData.m_shader->bind();
		
		s_MeshData.camera_buffer_mesh.view_projection3D = camera.getProjection() * glm::inverse(transform);
		s_MeshData.camera_buffer_mesh.camPos = glm::vec4(transform[3][0], transform[3][1], transform[3][2], 1.0f);
		s_MeshData.camera_uniform_buffer_mesh->setData(&s_MeshData.camera_buffer_mesh, sizeof(Renderer_Storeage::CameraData));
		s_MeshData.light_vp_uniform_buffer_shadowmap->setData(&light_vp, sizeof(MeshRendererStoreage::LightVPData));

		s_MeshData.m_scene_meshes.clear();
	}

	void MeshRenderer::BeginSceneMesh(EditorCamera& camera, const glm::mat4& light_vp) {
		s_MeshData.m_shader->bind();

		s_MeshData.light_vp_uniform_buffer_shadowmap->setData(&light_vp, sizeof(MeshRendererStoreage::LightVPData));

		s_MeshData.camera_buffer_mesh.view_projection3D = camera.getViewProjection();
		auto camPos = camera.getPosition();
		s_MeshData.camera_buffer_mesh.camPos = glm::vec4(camPos.x, camPos.y, camPos.z, 1.0f);
		s_MeshData.camera_uniform_buffer_mesh->setData(&s_MeshData.camera_buffer_mesh, sizeof(MeshRendererStoreage::CameraData));

		s_MeshData.m_scene_meshes.clear();
	}

	void MeshRenderer::EndShadowMapPass() {
		FlushMeshShadowMapPass();
		s_MeshData.m_scene_meshes.clear();
	}

	void MeshRenderer::EndSceneMesh(uint32_t shadowmap) {
		for (size_t i = 0; i < s_Data.scene_plights; i++) {
			s_MeshData.plights_buffer_mesh.point_lights[i] = s_Data.point_lights[i];
		}
		s_MeshData.plights_buffer_mesh.nrLights = s_Data.scene_plights;

		s_MeshData.dlight_buffer_mesh.dlight = s_Data.skyLight;
		s_MeshData.dlight_uniform_buffer_mesh->setData(&s_MeshData.dlight_buffer_mesh, sizeof(MeshRendererStoreage::DirLightData));
		s_MeshData.plights_uniform_buffer_mesh->setData(&s_MeshData.plights_buffer_mesh, sizeof(MeshRendererStoreage::PointLightsData));
		FlushMesh(shadowmap);
		s_MeshData.m_scene_meshes.clear();
	}

	void MeshRenderer::drawMesh(const glm::mat4& transform,MeshComponent& meshcomp, int entityID) {
		if (meshcomp.path == "") return;

		bool mesh_is_ready = false;
		std::unique_lock<std::mutex> lock(g_modelLoadMutex);
		auto iterator = s_MeshData.m_stored_meshes.find(meshcomp.path);
		lock.unlock();
		if (iterator == s_MeshData.m_stored_meshes.end()) {
			if (g_pendingLoads.find(meshcomp.path) == g_pendingLoads.end()) {
				// Trimitem un task de încărcare
				ModelLoadTask task;
				task.path = meshcomp.path;
				task.entityID = entityID;
				std::promise<void> p;
				std::shared_future<void> future = p.get_future().share();
				task.donePromise = std::move(p);
				g_pendingLoads[meshcomp.path] = future;

				{
					std::lock_guard<std::mutex> lock(g_modelLoadMutex);
					g_modelLoadQueue.push(std::move(task));
				}
				g_modelLoadCV.notify_one();
			}
		}
		
		//if (!meshcomp.first_pass && meshcomp.initialized == false && iterator != s_MeshData.m_stored_meshes.end()) {
		//	// Setăm materialele dacă modelul era deja încărcat
		//	meshcomp.materials.clear();
		//	for (auto& mat : s_MeshData.m_stored_meshes[meshcomp.path].materials) {
		//		meshcomp.materials.push_back(mat);
		//	}
		//	meshcomp.first_pass = true;
		//	meshcomp.initialized = true;
		//	mesh_is_ready = true;
		//}
		
		if (!meshcomp.first_pass && iterator != s_MeshData.m_stored_meshes.end() && meshcomp.initialized == false) {
			Mesh& stored_mesh = s_MeshData.m_stored_meshes[meshcomp.path];

			IARA_CORE_TRACE("STARTING TO CREATE THE MESH MATERIALS");
			stored_mesh.createMaterials();

			IARA_CORE_TRACE("STARTING TO CREATE THE MESH BUFFERS!");
			stored_mesh.createBuffers();
			stored_mesh.vb->SetData(s_MeshData.m_stored_meshes[meshcomp.path].mesh_vertex_array.data(), sizeof(MeshVertex) * (uint32_t)s_MeshData.m_stored_meshes[meshcomp.path].mesh_vertex_array.size());
			stored_mesh.ib->setData(s_MeshData.m_stored_meshes[meshcomp.path].indices.data(), (uint32_t)s_MeshData.m_stored_meshes[meshcomp.path].indices.size());

			stored_mesh.vao->setVertexBuffer(s_MeshData.m_stored_meshes[meshcomp.path].vb);
			stored_mesh.vao->SetIndexBuffer(s_MeshData.m_stored_meshes[meshcomp.path].ib);

			if (meshcomp.materials.empty()) {
				meshcomp.materials = stored_mesh.materials;
			}
			else {
				stored_mesh.materials = meshcomp.materials;
			}

			meshcomp.first_pass = true;
			mesh_is_ready = true;
			meshcomp.initialized = true;
		}

		
		if (meshcomp.initialized) {
			// Adăugăm mesh-ul în lista de randare (asumăm că e încărcat acum)
			std::vector<int> entityIDBuffer(s_MeshData.m_stored_meshes[meshcomp.path].m_num_vertices, entityID);
			SceneMeshData smd;
			smd.path = meshcomp.path;
			smd.materials = meshcomp.materials;
			smd.transform = transform;
			smd.entityID_VB = VertexBuffer::Create((void*)entityIDBuffer.data(), entityIDBuffer.size() * sizeof(int));
			smd.entityID_VB->setLayout({
				{ ShaderDataType::Int, "a_entity_id" }
				});

			s_MeshData.m_scene_meshes.push_back(smd);
		}

		
	}

	void MeshRenderer::FlushMeshShadowMapPass() {
		s_MeshData.m_vao->bind();
		for (auto& mesh_entry : s_MeshData.m_scene_meshes) {
			auto& raw_mesh_data = s_MeshData.m_stored_meshes[mesh_entry.path];
			s_MeshData.model_uniform_buffer_shadowmap->setData(&mesh_entry.transform, sizeof(MeshRendererStoreage::ModelData));
			/// Setting data in the vertex buffer AND index buffer

			s_MeshData.m_vao->setVertexBuffer(raw_mesh_data.vb);
			s_MeshData.m_vao->SetIndexBuffer(raw_mesh_data.ib);

			for (auto& mesh : raw_mesh_data.meshes) {
				RenderCommand::DrawIndexedBaseVertex(s_MeshData.m_vao, mesh.numInd, mesh.baseIndex, mesh.baseVertex);
			}
		}
	}

	void MeshRenderer::FlushMesh(uint32_t shadowmap) {
		s_MeshData.m_vao->bind();
		for (auto &mesh_entry : s_MeshData.m_scene_meshes) {
			auto &raw_mesh_data = s_MeshData.m_stored_meshes[mesh_entry.path];
			s_MeshData.model_uniform_buffer_mesh->setData(&mesh_entry.transform, sizeof(MeshRendererStoreage::ModelData));			
			/// Setting data in the vertex buffer AND index buffer

			s_MeshData.m_vao->setVertexBuffer(raw_mesh_data.vb);
			s_MeshData.m_vao->SetIndexBuffer(raw_mesh_data.ib);

			//// **Create an Entity ID buffer for this specific instance (all vertices share the same entityID)**
			/// AYOOOOO IT FUCKING WORKES
			/// VALID ENTITYIDS FOR EVERYONE YOOHOOOO
			s_MeshData.m_vao->AddVertexBuffer(mesh_entry.entityID_VB);

			for (auto &mesh : raw_mesh_data.meshes) {
				uint8_t tex_slot = 0;
				ShaderMaterial sh_mat;
				sh_mat.albedo = mesh_entry.materials[mesh.materialInd].diffuse;
				sh_mat.shininess = mesh_entry.materials[mesh.materialInd].shininess;
				s_MeshData.materials_uniform_buffer_mesh->setData(&sh_mat, sizeof(ShaderMaterial));

				Material& material = mesh_entry.materials[mesh.materialInd];

				material.diffuse_map->bind(tex_slot);
				s_MeshData.m_shader->setUniformInt("diffuse_map", tex_slot);
				tex_slot++;

				material.specular_map->bind(tex_slot);
				s_MeshData.m_shader->setUniformInt("specular_map", tex_slot);
				tex_slot++;

				material.normal_map->bind(tex_slot);
				s_MeshData.m_shader->setUniformInt("normal_map", tex_slot);
				tex_slot++;

				RenderCommand::BindTextureUnit(tex_slot, shadowmap);
				s_MeshData.m_shader->setUniformInt("shadow_map", tex_slot);
				tex_slot++;

				RenderCommand::DrawIndexedBaseVertex(s_MeshData.m_vao, mesh.numInd, mesh.baseIndex, mesh.baseVertex);
			}
		}
	}

}
