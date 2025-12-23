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
#include <random>
#include <condition_variable>

#include <glm/ext/matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm/glm.hpp>

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

		/// 
		Ref<VertexBuffer> heightmap_vb;
		Ref<VertexArray> heightmap_vao;
		Ref<IndexBuffer> heightmap_ib;

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

		Ref<UniformBuffer> heightmap_ubo;
	};

	struct ShadowMapData {
		Ref<VertexArray> shadow_quad_vao;
		Ref<VertexBuffer> shadow_quad_vb;
		Ref<IndexBuffer> shadow_quad_ib;

		Ref<Shader> quad_shadowmap_shader;
	};

	struct AtmosphereData {
		Ref<Shader> atmosphere_shader;

		struct AtmData {
			glm::vec4 camera;
			glm::vec4 white_point;
			glm::vec4 earth_center;
			glm::vec4 sun_direction;
			glm::vec4 sun_size;
			glm::vec4 viewport_size;
			float exposure;
		};

		struct AtmViewData {
			glm::mat4 model_from_view;
			glm::mat4 view_from_clip;
		};

		AtmData atm_data_buffer;
		Ref<UniformBuffer> atm_data_uniform_buffer;

		AtmViewData atm_view_data_buffer;
		Ref<UniformBuffer> atm_view_data_unifor_buffer;
	};

	struct HDRData {
		Ref<Shader> quad_ldr_shader;

		Ref<UniformBuffer> exposure_ubo;
	};

	static Renderer_Storeage s_Data;
	static HDRData s_hdrData;
	static ShadowMapData s_shadowMapData;
	static AtmosphereData s_atmosphereData;

	static Ref<ShaderLibrary> s_shaderLibrary = std::make_shared<ShaderLibrary>();

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

		s_Data.heightmap_vao = VertexArray::Create();
		int gridW = 848;
		int gridH = 480;

		std::vector<glm::vec2> vertices;
		std::vector<unsigned int> indices;

		// Build vertices (UVs in [0,1])
		for (int y = 0; y < gridH; y++) {
			for (int x = 0; x < gridW; x++) {
				float u = (float)x / (gridW - 1);
				float v = (float)y / (gridH - 1);
				vertices.push_back(glm::vec2(u, v));
			}
		}

		// Build indices (two triangles per quad)
		for (int y = 0; y < gridH - 1; y++) {
			for (int x = 0; x < gridW - 1; x++) {
				int i0 = y * gridW + x;
				int i1 = i0 + 1;
				int i2 = i0 + gridW;
				int i3 = i2 + 1;

				// triangle 1: i0, i2, i1
				indices.push_back(i0);
				indices.push_back(i2);
				indices.push_back(i1);

				// triangle 2: i1, i2, i3
				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);
			}
		}
		s_Data.heightmap_ib = IndexBuffer::Create(indices.data(), indices.size());
		s_Data.heightmap_vb = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(glm::vec2));
		s_Data.heightmap_vao->setVertexBuffer(s_Data.heightmap_vb);
		s_Data.heightmap_vao->SetIndexBuffer(s_Data.heightmap_ib);

		s_shaderLibrary->load("lumen", "Shaders/depth_lumen_combined.vert", "Shaders/depth_lumen_combined.frag");
		s_shaderLibrary->load("lumen-h", "Shaders/heightmap_gen.vert", "Shaders/heightmap_gen.frag");
		s_shadowMapData.quad_shadowmap_shader = Shader::Create("shadowMapQuad", "Shaders/shadowmap_quad.vert", "Shaders/shadowmap_quad.frag");
		s_shadowMapData.shadow_quad_vb = VertexBuffer::Create(quadVertices, sizeof(float) * 30);

		s_shadowMapData.shadow_quad_vb->setLayout({
			{ ShaderDataType::Float3, "a_pos" },
			{ ShaderDataType::Float2, "a_tex_coord" }
			});
		s_shadowMapData.shadow_quad_vao->AddVertexBuffer(s_shadowMapData.shadow_quad_vb);

		///-------------

		s_hdrData.quad_ldr_shader = Shader::Create("hdrToneMapping", "Shaders/tone_mapping.vert", "Shaders/tone_mapping.frag");
		s_hdrData.exposure_ubo = UniformBuffer::Create(sizeof(float), 21);

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
		s_Data.heightmap_ubo = UniformBuffer::Create(sizeof(glm::mat4), 26);


		// Atmosphere
		s_shaderLibrary->load("atmosphere", "Shaders/atm.vert", "Shaders/atm.frag");
		s_atmosphereData.atm_data_uniform_buffer = UniformBuffer::Create(sizeof(AtmosphereData::AtmData), 24);
		s_atmosphereData.atm_view_data_unifor_buffer = UniformBuffer::Create(sizeof(AtmosphereData::AtmViewData), 23);
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

	void Renderer2D::drawQuadTBillboard(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& color, EditorCamera& camera, float tiling_mult, int entityID) {
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

	void Renderer2D::applyToneMapping(uint32_t hdr_texture, float exposure) {
		s_hdrData.quad_ldr_shader->bind();
		s_shadowMapData.shadow_quad_vao->bind();

		RenderCommand::BindTextureUnit(0, hdr_texture);
		s_hdrData.quad_ldr_shader->setUniformInt("hdr_texture", 0);
		s_hdrData.exposure_ubo->setData(&exposure, sizeof(float));

		RenderCommand::drawArrays(s_shadowMapData.shadow_quad_vao, 0, 7);

		s_hdrData.quad_ldr_shader->unbind();
	}

	void Renderer2D::renderLumen(uint32_t tex2198, uint32_t tex2199, uint32_t tex2200) {
		s_shaderLibrary->get("lumen")->bind();
		s_shadowMapData.shadow_quad_vao->bind();
		RenderCommand::BindTextureUnit(0, tex2198);
		RenderCommand::BindTextureUnit(1, tex2199);
		RenderCommand::BindTextureUnit(2, tex2200);
		s_shaderLibrary->get("lumen")->setUniformInt("tex2198", 0);
		s_shaderLibrary->get("lumen")->setUniformInt("tex2199", 1);
		s_shaderLibrary->get("lumen")->setUniformInt("tex2200", 2);
		RenderCommand::drawArrays(s_shadowMapData.shadow_quad_vao, 0, 7);
		s_shaderLibrary->get("lumen")->unbind();



		/*s_shaderLibrary->get("lumen-h")->bind();
		s_Data.heightmap_vao->bind();
		glm::mat4 mvp = glm::perspective(glm::radians(45.0f), 848.0f / 480.0f, 0.1f, 10000.0f) * glm::mat4(1.0f);
		s_Data.heightmap_ubo->setData(&mvp, sizeof(glm::mat4));
		RenderCommand::BindTextureUnit(0, tex2198);
		s_shaderLibrary->get("lumen-h")->setUniformInt("uDepth", 0);
		RenderCommand::DrawIndexed(s_Data.heightmap_vao, s_Data.heightmap_ib->getCount());
		s_shaderLibrary->get("lumen-h")->unbind();*/
	}

	void Renderer2D::drawAtmosphere(uint32_t vp_width, uint32_t vp_height, const glm::mat4& model_from_view, const glm::mat4& view_from_clip, const glm::vec3& camera_pos, const glm::vec3& white_point, const glm::vec3& earth_center, const glm::vec3& sun_dir,
									const glm::vec2& sun_size, float exposure, uint32_t transmittance_tex, uint32_t scattering_tex,
									uint32_t mie_scattering_tex, uint32_t irradiance_tex, uint32_t lighting_pass_tex, uint32_t g_depth, uint32_t g_position) {

		s_shaderLibrary->get("atmosphere")->bind();
		//this_will_dissappear::bindShader(shader);
		//glUseProgram(shader);

		s_atmosphereData.atm_data_buffer.camera = glm::vec4(camera_pos, 0.0);
		s_atmosphereData.atm_data_buffer.earth_center = glm::vec4(earth_center, 0.0);
		s_atmosphereData.atm_data_buffer.white_point = glm::vec4(white_point, 0.0);
		s_atmosphereData.atm_data_buffer.sun_direction = glm::vec4(sun_dir, 0.0);
		s_atmosphereData.atm_data_buffer.sun_size = glm::vec4(sun_size, 0.0, 0.0);
		s_atmosphereData.atm_data_buffer.viewport_size = glm::vec4(vp_width, vp_height, 0.0, 0.0);
		s_atmosphereData.atm_data_buffer.exposure = exposure;
		s_atmosphereData.atm_data_uniform_buffer->setData(&s_atmosphereData.atm_data_buffer, sizeof(AtmosphereData::AtmData));
		
		s_atmosphereData.atm_view_data_buffer.model_from_view = model_from_view;
		s_atmosphereData.atm_view_data_buffer.view_from_clip = view_from_clip;
		s_atmosphereData.atm_view_data_unifor_buffer->setData(&s_atmosphereData.atm_view_data_buffer, sizeof(AtmosphereData::AtmViewData));

		RenderCommand::ActiveBindTexture2D(0, transmittance_tex);
		s_shaderLibrary->get("atmosphere")->setUniformInt("transmittance_texture", 0);

		RenderCommand::ActiveBindTexture3D(1, scattering_tex);
		s_shaderLibrary->get("atmosphere")->setUniformInt("scattering_texture", 1);


		RenderCommand::ActiveBindTexture3D(2, mie_scattering_tex);
		s_shaderLibrary->get("atmosphere")->setUniformInt("single_mie_scattering_texture", 2);
		//this_will_dissappear::bindShaderTex(shader, "single_mie_scattering_texture", 2);


		//RenderCommand::BindTextureUnit(3, irradiance_tex);
		RenderCommand::ActiveBindTexture2D(3, irradiance_tex);
		s_shaderLibrary->get("atmosphere")->setUniformInt("irradiance_texture", 3);
		//this_will_dissappear::bindShaderTex(shader, "irradiance_texture", 3);

		RenderCommand::ActiveBindTexture2D(4, lighting_pass_tex);
		s_shaderLibrary->get("atmosphere")->setUniformInt("lighting_pass_texture", 4);

		RenderCommand::ActiveBindTexture2D(5, g_depth);
		s_shaderLibrary->get("atmosphere")->setUniformInt("g_depth_texture", 5);

		RenderCommand::ActiveBindTexture2D(6, g_position);
		s_shaderLibrary->get("atmosphere")->setUniformInt("g_position_texture", 6);

		s_shadowMapData.shadow_quad_vao->bind();
		RenderCommand::drawArrays(s_shadowMapData.shadow_quad_vao, 0, 6);
		s_shaderLibrary->get("atmosphere")->unbind();
	}

	void Renderer2D::drawFullScreenQuad(const std::vector<bool>& enable_blend) {
		for (unsigned int i = 0; i < enable_blend.size(); ++i) {
			if (enable_blend[i]) {
				RenderCommand::BlendEnablei(i);
			}
		}

		// shader bound outside of this
		s_shadowMapData.shadow_quad_vao->bind();
		RenderCommand::drawArraysStrip(0, 4);

		for (unsigned int i = 0; i < enable_blend.size(); ++i) {
			RenderCommand::BlendDisablei(i);
		}
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
		Ref<Texture2D> noise_map;
		Ref<Texture2D> m_cubemap;
		Ref<VertexArray> vao_cubemap;
		Ref<VertexBuffer> vb_cubemap;

		struct CameraData_skybox {
			glm::mat4 view_projection3D;
		};

		struct DynamicSkyResources {
			glm::vec2 resolution;
			glm::vec2 mouse_pos;
			float time;
		};

		CameraData_skybox camera_buffer_skybox;
		Ref<UniformBuffer> camera_uniform_buffer_skybox;

		Ref<UniformBuffer> sun_direction_uniform_buffer_skybox;
		DynamicSkyResources dynamic_sky_resources;
		Ref<UniformBuffer> dynamic_sky_uniform_buffer;
	};

	static CubeMap_Resources s_cubemap;

	void Renderer3D::Init3D() {
		s_cubemap.m_cubemap = Texture2D::CreateCubemap("Assets/Textures/skybox2/sky2.png");
		

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

		s_cubemap.sun_direction_uniform_buffer_skybox = UniformBuffer::Create(sizeof(glm::vec4), 17);
		s_cubemap.dynamic_sky_uniform_buffer = UniformBuffer::Create(sizeof(CubeMap_Resources::DynamicSkyResources), 25);
	}

	void Renderer3D::drawSkyBox(const glm::mat4& view_proj, const Ref<Texture2D>& skybox, const glm::vec4& direction) {
		RenderCommand::setDepthMask(false);
		s_shaderLibrary->get("cubemap")->bind();

		skybox->bind();
		s_cubemap.camera_buffer_skybox.view_projection3D = view_proj;
		s_cubemap.camera_uniform_buffer_skybox->setData(&s_cubemap.camera_buffer_skybox, sizeof(CubeMap_Resources::CameraData_skybox));

		s_cubemap.sun_direction_uniform_buffer_skybox->setData(&direction, sizeof(glm::vec4));

		s_cubemap.vao_cubemap->bind();
		RenderCommand::drawArrays(s_cubemap.vao_cubemap, 0, 36);
		RenderCommand::setDepthMask(true);
		s_shaderLibrary->get("cubemap")->unbind();
	}

	void Renderer3D::drawDynamicSky(const glm::vec2& resolution, const glm::vec2& mouse_pos, float time) {
		RenderCommand::setDepthMask(false);
		s_shaderLibrary->get("dynamic-sky")->bind();
		s_cubemap.dynamic_sky_resources.resolution = resolution;
		s_cubemap.dynamic_sky_resources.mouse_pos = mouse_pos;
		s_cubemap.dynamic_sky_resources.time = time;

		s_cubemap.dynamic_sky_uniform_buffer->setData(&s_cubemap.dynamic_sky_resources, sizeof(CubeMap_Resources::DynamicSkyResources));
		//s_cubemap.vao_cubemap->bind();
		s_shadowMapData.shadow_quad_vao->bind();
		RenderCommand::BindTextureUnit(1, s_cubemap.noise_map->getRendererID());
		s_shaderLibrary->get("dynamic-sky")->setUniformInt("iChannel0", 1);

		RenderCommand::drawArrays(s_shadowMapData.shadow_quad_vao, 0, 6);
		RenderCommand::setDepthMask(true);
		s_shaderLibrary->get("dynamic-sky")->unbind();
	}


	// 
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


	struct MeshRendererStoreage {
		Ref<VertexArray> vao;

		Ref<ShaderLibrary> s_shaderLibrary;

		Ref<Texture2D> ssao_noise_tex;
		std::vector<glm::vec4> ssao_kernel;

		std::unordered_map<std::string, Mesh> stored_meshes;
		std::vector<SceneMeshData> scene_meshes;

		struct CameraData {
			glm::mat4 view_projection3D;
			glm::mat4 view;
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

		Ref<UniformBuffer> ssao_random_samples_uniform_buffer;
		Ref<UniformBuffer> ssao_projection_uniform_buffer;
		Ref<UniformBuffer> viewport_sizes_uniform_buffer;

		Ref<UniformBuffer> ambient_variables_uniform_buffer;

	};

	static MeshRendererStoreage s_MeshData;

	

	void MeshRenderer::InitMeshRenderer() {
		
		s_shaderLibrary->load("shadowmap", "Shaders/shadowmap.vert", "Shaders/shadowmap.frag");
		s_shaderLibrary->load("gbuffer", "Shaders/deferred_geometry_pass.vert", "Shaders/deferred_geometry_pass.frag");
		s_shaderLibrary->load("cubemap", "Shaders/cubemap.vert", "Shaders/cubemap.frag");
		s_shaderLibrary->load("dynamic-sky", "Shaders/AtmosLightScatter.vert", "Shaders/AtmosLightScatter.frag");
		s_shaderLibrary->load("ssao_texture", "Shaders/ssao_texture.vert", "Shaders/ssao_texture.frag");
		s_shaderLibrary->load("ssao_blur_texture", "Shaders/ssao_blur_texture.vert", "Shaders/ssao_blur_texture.frag");
		s_shaderLibrary->load("lighting_pass", "Shaders/deferred_lighting_pass.vert", "Shaders/deferred_lighting_pass.frag");

		s_MeshData.ssao_noise_tex = Texture2D::Create("Assets/Textures/noise-tex.png");

		s_MeshData.vao = VertexArray::Create();
		s_MeshData.camera_uniform_buffer_mesh = UniformBuffer::Create(sizeof(MeshRendererStoreage::CameraData), 6);
		s_MeshData.model_uniform_buffer_mesh = UniformBuffer::Create(sizeof(MeshRendererStoreage::ModelData), 7);
		s_MeshData.materials_uniform_buffer_mesh = UniformBuffer::Create(sizeof(MeshRendererStoreage::MaterialsData), 8);

		s_MeshData.plights_uniform_buffer_mesh = UniformBuffer::Create(sizeof(Renderer_Storeage::PointLightsData), 9);
		s_MeshData.dlight_uniform_buffer_mesh = UniformBuffer::Create(sizeof(Renderer_Storeage::DirLightData), 10);

		s_MeshData.light_vp_uniform_buffer_shadowmap = UniformBuffer::Create(sizeof(MeshRendererStoreage::LightVPData), 11);
		s_MeshData.model_uniform_buffer_shadowmap    = UniformBuffer::Create(sizeof(MeshRendererStoreage::ModelData), 12);

		s_MeshData.ssao_random_samples_uniform_buffer = UniformBuffer::Create(sizeof(glm::vec4) * 64, 13);
		s_MeshData.ssao_projection_uniform_buffer = UniformBuffer::Create(sizeof(glm::mat4) * 2, 14);
		s_MeshData.viewport_sizes_uniform_buffer = UniformBuffer::Create(sizeof(glm::vec2), 15);
		s_MeshData.ambient_variables_uniform_buffer = UniformBuffer::Create(sizeof(bool), 16);

	}

	void MeshRenderer::Shutdown() {
	
	}

	Ref<ShaderLibrary> MeshRenderer::getShaderLibrary() {
		return s_shaderLibrary;
	}

	void MeshRenderer::ShadowMapPass(const glm::mat4& transform) {
		s_shaderLibrary->get("shadowmap")->bind();

		MeshRendererStoreage::LightVPData data;
		data.light_view_projection = transform;

		s_MeshData.light_vp_uniform_buffer_shadowmap->setData(&data, sizeof(MeshRendererStoreage::LightVPData));
		
		FlushMeshGeometryPassShadowMap();
	}

	void MeshRenderer::ForwardPass(const Camera& camera, const glm::mat4& transform, const glm::mat4& light_vp, uint32_t shadowmap) {
		s_shaderLibrary->get("mesh_light")->bind();

		s_MeshData.camera_buffer_mesh.view_projection3D = camera.getProjection() * glm::inverse(transform);
		s_MeshData.camera_buffer_mesh.camPos = glm::vec4(transform[3][0], transform[3][1], transform[3][2], 1.0f);
		//s_MeshData.camera_buffer_mesh.view = camera.getViewMatrix();
		s_MeshData.camera_uniform_buffer_mesh->setData(&s_MeshData.camera_buffer_mesh, sizeof(Renderer_Storeage::CameraData));
		s_MeshData.light_vp_uniform_buffer_shadowmap->setData(&light_vp, sizeof(MeshRendererStoreage::LightVPData));

		for (size_t i = 0; i < s_Data.scene_plights; i++) {
			s_MeshData.plights_buffer_mesh.point_lights[i] = s_Data.point_lights[i];
		}
		s_MeshData.plights_buffer_mesh.nrLights = s_Data.scene_plights;

		s_MeshData.dlight_buffer_mesh.dlight = s_Data.skyLight;
		s_MeshData.dlight_uniform_buffer_mesh->setData(&s_MeshData.dlight_buffer_mesh, sizeof(MeshRendererStoreage::DirLightData));
		s_MeshData.plights_uniform_buffer_mesh->setData(&s_MeshData.plights_buffer_mesh, sizeof(MeshRendererStoreage::PointLightsData));

		FlushMesh(shadowmap);
	}

	void MeshRenderer::ForwardPass(EditorCamera& camera, const glm::mat4& light_vp, uint32_t shadowmap) {
		s_shaderLibrary->get("mesh_light")->bind();

		s_MeshData.light_vp_uniform_buffer_shadowmap->setData(&light_vp, sizeof(MeshRendererStoreage::LightVPData));

		s_MeshData.camera_buffer_mesh.view_projection3D = camera.getViewProjection();
		auto camPos = camera.getPosition();
		s_MeshData.camera_buffer_mesh.camPos = glm::vec4(camPos.x, camPos.y, camPos.z, 1.0f);
		s_MeshData.camera_buffer_mesh.view = camera.getViewMatrix();
		s_MeshData.camera_uniform_buffer_mesh->setData(&s_MeshData.camera_buffer_mesh, sizeof(MeshRendererStoreage::CameraData));

		for (size_t i = 0; i < s_Data.scene_plights; i++) {
			s_MeshData.plights_buffer_mesh.point_lights[i] = s_Data.point_lights[i];
		}
		s_MeshData.plights_buffer_mesh.nrLights = s_Data.scene_plights;

		s_MeshData.dlight_buffer_mesh.dlight = s_Data.skyLight;
		s_MeshData.dlight_uniform_buffer_mesh->setData(&s_MeshData.dlight_buffer_mesh, sizeof(MeshRendererStoreage::DirLightData));
		s_MeshData.plights_uniform_buffer_mesh->setData(&s_MeshData.plights_buffer_mesh, sizeof(MeshRendererStoreage::PointLightsData));

		FlushMesh(shadowmap);
	}

	void MeshRenderer::GeometryPassGBuffer(EditorCamera& camera) {
		s_shaderLibrary->get("gbuffer")->bind();

		struct camera_data {
			glm::mat4 view;
			glm::mat4 proj;
		} view_proj;
		view_proj.view = camera.getViewMatrix();
		view_proj.proj = camera.getProjection();
		s_MeshData.ssao_projection_uniform_buffer->setData(&view_proj, sizeof(glm::mat4) * 2);

		FlushMeshGeometryPassShadowMap();
	}

	void MeshRenderer::GeometryPassGBuffer(const Camera& camera, const glm::mat4& transform) {
		s_shaderLibrary->get("gbuffer")->bind();

		struct camera_data {
			glm::mat4 view;
			glm::mat4 proj;
		} view_proj;
		view_proj.view = glm::inverse(transform);
		view_proj.proj = camera.getProjection();
		s_MeshData.ssao_projection_uniform_buffer->setData(&view_proj, sizeof(glm::mat4) * 2);

		FlushMeshGeometryPassShadowMap();
	}

	void MeshRenderer::SSAOPass(EditorCamera& camera, uint32_t vp_width, uint32_t vp_height, uint32_t gposition, uint32_t gnormal, uint32_t entityID_map) {
		s_shaderLibrary->get("ssao_texture")->bind();

		/// UBO
		s_MeshData.ssao_random_samples_uniform_buffer->setData(s_MeshData.ssao_kernel.data(), sizeof(glm::vec4) * s_MeshData.ssao_kernel.size());

		struct camera_data {
			glm::mat4 view;
			glm::mat4 proj;
		} view_proj;
		view_proj.view = camera.getViewMatrix();
		view_proj.proj = camera.getProjection();
		s_MeshData.ssao_projection_uniform_buffer->setData(&view_proj, sizeof(glm::mat4) * 2);

		struct VPSizes {
			float x, y;
		} sizes;
		sizes.x = (float)vp_width;
		sizes.y = (float)vp_height;
		s_MeshData.viewport_sizes_uniform_buffer->setData(&sizes, sizeof(float) * 2);
		/// UBO END


		RenderCommand::BindTextureUnit(0, gposition);
		s_shaderLibrary->get("ssao_texture")->setUniformInt("gPosition", 0);
		RenderCommand::BindTextureUnit(1, gnormal);
		s_shaderLibrary->get("ssao_texture")->setUniformInt("gNormal", 1);
		RenderCommand::BindTextureUnit(2, s_MeshData.ssao_noise_tex->getRendererID());
		s_shaderLibrary->get("ssao_texture")->setUniformInt("noiseTexture", 2);
		RenderCommand::BindTextureUnit(3, entityID_map);
		s_shaderLibrary->get("ssao_texture")->setUniformInt("entityID_map", 3);

		s_shadowMapData.shadow_quad_vao->bind();
		RenderCommand::drawArrays(s_shadowMapData.shadow_quad_vao, 0, 6);

		s_shaderLibrary->get("ssao_texture")->unbind();
	}

	void MeshRenderer::SSAOPass(const Camera& camera, const glm::mat4& transform, uint32_t vp_width, uint32_t vp_height, uint32_t gposition, uint32_t gnormal, uint32_t entityID_map) {
		s_shaderLibrary->get("ssao_texture")->bind();

		/// UBO
		s_MeshData.ssao_random_samples_uniform_buffer->setData(s_MeshData.ssao_kernel.data(), sizeof(glm::vec4) * s_MeshData.ssao_kernel.size());

		struct camera_data {
			glm::mat4 view;
			glm::mat4 proj;
		} view_proj;
		view_proj.view = glm::inverse(transform);
		view_proj.proj = camera.getProjection();
		s_MeshData.ssao_projection_uniform_buffer->setData(&view_proj, sizeof(glm::mat4) * 2);

		struct VPSizes {
			float x, y;
		} sizes;
		sizes.x = (float)vp_width;
		sizes.y = (float)vp_height;
		s_MeshData.viewport_sizes_uniform_buffer->setData(&sizes, sizeof(float) * 2);
		/// UBO END


		RenderCommand::BindTextureUnit(0, gposition);
		s_shaderLibrary->get("ssao_texture")->setUniformInt("gPosition", 0);
		RenderCommand::BindTextureUnit(1, gnormal);
		s_shaderLibrary->get("ssao_texture")->setUniformInt("gNormal", 1);
		RenderCommand::BindTextureUnit(2, s_MeshData.ssao_noise_tex->getRendererID());
		s_shaderLibrary->get("ssao_texture")->setUniformInt("noiseTexture", 2);
		RenderCommand::BindTextureUnit(3, entityID_map);
		s_shaderLibrary->get("ssao_texture")->setUniformInt("entityID_map", 3);

		s_shadowMapData.shadow_quad_vao->bind();
		RenderCommand::drawArrays(s_shadowMapData.shadow_quad_vao, 0, 7);

		s_shaderLibrary->get("ssao_texture")->unbind();
	}

	void MeshRenderer::SSAOBlurPass(uint32_t ssao_input) {
		s_shaderLibrary->get("ssao_blur_texture")->bind();

		RenderCommand::BindTextureUnit(0, ssao_input);
		s_shaderLibrary->get("ssao_blur_texture")->setUniformInt("ssao_input", 0);

		s_shadowMapData.shadow_quad_vao->bind();
		RenderCommand::drawArrays(s_shadowMapData.shadow_quad_vao, 0, 7);

		s_shaderLibrary->get("ssao_blur_texture")->unbind();
	}

	void MeshRenderer::LighintgPass(EditorCamera& camera, uint32_t gposition, uint32_t gnormal, uint32_t gdiffusespec, uint32_t entityID_map, uint32_t shadowmap, uint32_t ssao_map, const glm::mat4& light_vp, bool ssao_state) {
		s_MeshData.scene_meshes.clear();
		s_shaderLibrary->get("lighting_pass")->bind();

		s_MeshData.camera_buffer_mesh.view_projection3D = camera.getViewProjection();
		auto camPos = camera.getPosition();
		s_MeshData.camera_buffer_mesh.camPos = glm::vec4(camPos.x, camPos.y, camPos.z, 1.0f);
		s_MeshData.camera_buffer_mesh.view = camera.getViewMatrix();
		s_MeshData.camera_uniform_buffer_mesh->setData(&s_MeshData.camera_buffer_mesh, sizeof(MeshRendererStoreage::CameraData));
		s_MeshData.light_vp_uniform_buffer_shadowmap->setData(&light_vp, sizeof(MeshRendererStoreage::LightVPData));

		for (size_t i = 0; i < s_Data.scene_plights; i++) {
			s_MeshData.plights_buffer_mesh.point_lights[i] = s_Data.point_lights[i];
		}
		s_MeshData.plights_buffer_mesh.nrLights = s_Data.scene_plights;

		s_MeshData.dlight_buffer_mesh.dlight = s_Data.skyLight;
		s_MeshData.dlight_uniform_buffer_mesh->setData(&s_MeshData.dlight_buffer_mesh, sizeof(MeshRendererStoreage::DirLightData));
		s_MeshData.plights_uniform_buffer_mesh->setData(&s_MeshData.plights_buffer_mesh, sizeof(MeshRendererStoreage::PointLightsData));

		bool ssao_enabled = ssao_state;
		s_MeshData.ambient_variables_uniform_buffer->setData(&ssao_enabled, sizeof(bool));

		s_shadowMapData.shadow_quad_vao->bind();

		RenderCommand::BindTextureUnit(0, gposition);
		s_shaderLibrary->get("lighting_pass")->setUniformInt("gPosition", 0);
		RenderCommand::BindTextureUnit(1, gnormal);
		s_shaderLibrary->get("lighting_pass")->setUniformInt("gNormal", 1);
		RenderCommand::BindTextureUnit(2, gdiffusespec);
		s_shaderLibrary->get("lighting_pass")->setUniformInt("gDiffuseSpec", 2);
		RenderCommand::BindTextureUnit(3, shadowmap);
		s_shaderLibrary->get("lighting_pass")->setUniformInt("shadow_map", 3);
		RenderCommand::BindTextureUnit(4, entityID_map);
		s_shaderLibrary->get("lighting_pass")->setUniformInt("entityID_map", 4);
		RenderCommand::BindTextureUnit(5, ssao_map);
		s_shaderLibrary->get("lighting_pass")->setUniformInt("ssao_map", 5);

		RenderCommand::drawArrays(s_shadowMapData.shadow_quad_vao, 0, 7);

		s_shaderLibrary->get("lighting_pass")->unbind();

		s_MeshData.scene_meshes.clear();
	}

	//void MeshRenderer::LighintgPass(const Camera& camera, const glm::mat4& transform, uint32_t gposition, uint32_t gnormal, uint32_t gdiffusespec, uint32_t entityID_map, uint32_t shadowmap, uint32_t ssao_map, const glm::mat4& light_vp, bool ssao_state) {
	//	s_shaderLibrary->get("lighting_pass")->bind();

	//	bool mesh_is_ready = false;
	//	std::unique_lock<std::mutex> lock(g_modelLoadMutex);
	//	auto iterator = s_MeshData.m_stored_meshes.find(meshcomp.path);
	//	lock.unlock();
	//	if (iterator == s_MeshData.m_stored_meshes.end()) {
	//		if (g_pendingLoads.find(meshcomp.path) == g_pendingLoads.end()) {
	//			// Trimitem un task de încărcare
	//			ModelLoadTask task;
	//			task.path = meshcomp.path;
	//			task.entityID = entityID;
	//			std::promise<void> p;
	//			std::shared_future<void> future = p.get_future().share();
	//			task.donePromise = std::move(p);
	//			g_pendingLoads[meshcomp.path] = future;

	//			{
	//				std::lock_guard<std::mutex> lock(g_modelLoadMutex);
	//				g_modelLoadQueue.push(std::move(task));
	//			}
	//			g_modelLoadCV.notify_one();
	//		}
	//	}
	//	
	//	//if (!meshcomp.first_pass && meshcomp.initialized == false && iterator != s_MeshData.m_stored_meshes.end()) {
	//	//	// Setăm materialele dacă modelul era deja încărcat
	//	//	meshcomp.materials.clear();
	//	//	for (auto& mat : s_MeshData.m_stored_meshes[meshcomp.path].materials) {
	//	//		meshcomp.materials.push_back(mat);
	//	//	}
	//	//	meshcomp.first_pass = true;
	//	//	meshcomp.initialized = true;
	//	//	mesh_is_ready = true;
	//	//}
	//	
	//	if (!meshcomp.first_pass && iterator != s_MeshData.m_stored_meshes.end() && meshcomp.initialized == false) {
	//		Mesh& stored_mesh = s_MeshData.m_stored_meshes[meshcomp.path];

	//		IARA_CORE_TRACE("STARTING TO CREATE THE MESH MATERIALS");
	//		stored_mesh.createMaterials();

	//		IARA_CORE_TRACE("STARTING TO CREATE THE MESH BUFFERS!");
	//		stored_mesh.createBuffers();
	//		stored_mesh.vb->SetData(s_MeshData.m_stored_meshes[meshcomp.path].mesh_vertex_array.data(), sizeof(MeshVertex) * (uint32_t)s_MeshData.m_stored_meshes[meshcomp.path].mesh_vertex_array.size());
	//		stored_mesh.ib->setData(s_MeshData.m_stored_meshes[meshcomp.path].indices.data(), (uint32_t)s_MeshData.m_stored_meshes[meshcomp.path].indices.size());

	//		stored_mesh.vao->setVertexBuffer(s_MeshData.m_stored_meshes[meshcomp.path].vb);
	//		stored_mesh.vao->SetIndexBuffer(s_MeshData.m_stored_meshes[meshcomp.path].ib);

	//		if (meshcomp.materials.empty()) {
	//			meshcomp.materials = stored_mesh.materials;
	//		}
	//		else {
	//			stored_mesh.materials = meshcomp.materials;
	//		}

	//		meshcomp.first_pass = true;
	//	}

	//	
	//	if (meshcomp.initialized) {
	//		// Adăugăm mesh-ul în lista de randare (asumăm că e încărcat acum)
	//		std::vector<int> entityIDBuffer(s_MeshData.m_stored_meshes[meshcomp.path].m_num_vertices, entityID);
	//		SceneMeshData smd;
	//		smd.path = meshcomp.path;
	//		smd.materials = meshcomp.materials;
	//		smd.transform = transform;
	//		smd.entityID_VB = VertexBuffer::Create((void*)entityIDBuffer.data(), entityIDBuffer.size() * sizeof(int));
	//		smd.entityID_VB->setLayout({
	//			{ ShaderDataType::Int, "a_entity_id" }
	//			});

	//		s_MeshData.m_scene_meshes.push_back(smd);
	//	}

	//	
	//}

	void MeshRenderer::FlushMeshGeometryPassShadowMap() {
		s_MeshData.vao->bind();
		for (auto& mesh_entry : s_MeshData.scene_meshes) {
			auto& raw_mesh_data = s_MeshData.stored_meshes[mesh_entry.path];
			s_MeshData.model_uniform_buffer_shadowmap->setData(&mesh_entry.transform, sizeof(MeshRendererStoreage::ModelData));
			/// Setting data in the vertex buffer AND index buffer

			s_MeshData.vao->setVertexBuffer(raw_mesh_data.vb);
			s_MeshData.vao->SetIndexBuffer(raw_mesh_data.ib);

			for (auto& mesh : raw_mesh_data.meshes) {
				RenderCommand::DrawIndexedBaseVertex(s_MeshData.vao, mesh.numInd, mesh.baseIndex, mesh.baseVertex);
			}
		}
	}

	void MeshRenderer::FlushMesh(uint32_t shadowmap) {
		s_MeshData.vao->bind();


		RenderCommand::BindTextureUnit(3, shadowmap);
		s_shaderLibrary->get("mesh_light")->setUniformInt("shadow_map", 3);

		for (auto& mesh_entry : s_MeshData.scene_meshes) {
			auto& raw_mesh_data = s_MeshData.stored_meshes[mesh_entry.path];
			s_MeshData.model_uniform_buffer_mesh->setData(&mesh_entry.transform, sizeof(MeshRendererStoreage::ModelData));
			/// Setting data in the vertex buffer AND index buffer

			s_MeshData.vao->setVertexBuffer(raw_mesh_data.vb);
			s_MeshData.vao->SetIndexBuffer(raw_mesh_data.ib);

			//// **Create an Entity ID buffer for this specific instance (all vertices share the same entityID)**
			/// AYOOOOO IT FUCKING WORKES
			/// VALID ENTITYIDS FOR EVERYONE YOOHOOOO
			s_MeshData.vao->AddVertexBuffer(mesh_entry.entityID_VB);

			for (auto& mesh : raw_mesh_data.meshes) {
				ShaderMaterial sh_mat;
				sh_mat.albedo = mesh_entry.materials[mesh.materialInd].diffuse;
				sh_mat.shininess = mesh_entry.materials[mesh.materialInd].shininess;
				s_MeshData.materials_uniform_buffer_mesh->setData(&sh_mat, sizeof(ShaderMaterial));

				Material& material = mesh_entry.materials[mesh.materialInd];

				material.diffuse_map->bind(0);
				s_shaderLibrary->get("mesh_light")->setUniformInt("diffuse_map", 0);

				material.specular_map->bind(1);
				s_shaderLibrary->get("mesh_light")->setUniformInt("specular_map", 1);

				material.normal_map->bind(2);
				s_shaderLibrary->get("mesh_light")->setUniformInt("normal_map", 2);

				RenderCommand::DrawIndexedBaseVertex(s_MeshData.vao, mesh.numInd, mesh.baseIndex, mesh.baseVertex);
			}
		}

		s_MeshData.scene_meshes.clear();
	}
}
