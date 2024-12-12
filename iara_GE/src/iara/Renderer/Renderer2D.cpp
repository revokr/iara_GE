#include "ir_pch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "shader.h"
#include "platform/openGL/OpenGLShader.h"
#include "iara\Renderer\UniformBuffer.h"
#include "RenderCommand.h"
#include "iara\Math\Math.h"

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
		int material_index;
	};

	struct Renderer_Storeage {

		uint32_t FRAMES = 0;

		const uint32_t MaxQuads = 20000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTexSlots = 32;

		const uint32_t MaxQuads3D = 20000;
		const uint32_t MaxVertices3D = MaxQuads3D * 4;
		const uint32_t MaxIndices3D = MaxQuads3D * 36;
		uint32_t current_material_index;
		uint32_t current_lights;
		Material material[MAX_MATERIALS];
		PointLight point_lights[MAX_LIGHTS];
		uint32_t scene_plights;
		DirLight skyLight;

		Ref<VertexArray> vao;
		Ref<VertexArray> vao3D;
		Ref<VertexBuffer> vertexBuffer;
		Ref<VertexBuffer> vertexBuffer3D;
		Ref<Shader> tex_shader;
		Ref<Shader> tex_shader3D;
		Ref<Texture2D> white_tex;

		uint32_t QuadIndCnt = 0;
		uint32_t CubeIndCnt = 0;
		QuadVertex* quadVertexBufferBase = nullptr;
		QuadVertex* quadVertexBufferPtr = nullptr;

		CubeVertex* cubeVertexBufferBase = nullptr;
		CubeVertex* cubeVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTexSlots> texture_slots;
		uint32_t textureSlotInd = 1; /// 0 = white texture

		glm::vec4 quadVertices[4];
		glm::vec4 cubeVertices[36];
		glm::vec3 cubeNormals[36];
		glm::vec2 texCoords[4];
		glm::vec2 cubeTexCoords[6];

		Statistics stats;

		struct CameraData {
			glm::mat4 view_projection3D;
			glm::vec4 camPos;
		};

		struct MaterialsData {
			Material material[MAX_MATERIALS];
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

		CameraData camera_buffer3D;
		Ref<UniformBuffer> camera_uniform_buffer3D;

		DirLightData dlight_buffer3D;
		Ref<UniformBuffer> dlight_uniform_buffer3D;
		
		MaterialsData materials_buffer3D;
		Ref<UniformBuffer> materials_uniform_buffer3D;

		PointLightsData plights_buffer3D;
		Ref<UniformBuffer> plights_uniform_buffer3D;
	};

	Renderer_Storeage s_Data;

	void Renderer2D::Init() {
		IARA_PROFILE_FUNCTION();

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

		/// 3D /////////////////////////////////////////

		s_Data.vao3D = (VertexArray::Create());

		s_Data.vertexBuffer3D = (VertexBuffer::Create(s_Data.MaxVertices3D * sizeof(CubeVertex)));

		s_Data.tex_shader3D = Shader::Create("texture3D-light", "Shaders/texture3-light.vert", "Shaders/texture3-light.frag");

		s_Data.vertexBuffer3D->setLayout({
			{ ShaderDataType::Float3, "a_pos" },
			{ ShaderDataType::Float3, "a_normal" },
			{ ShaderDataType::Float4, "a_color" },
			{ ShaderDataType::Float2, "a_tex" },
			{ ShaderDataType::Float,  "a_tex_id" },
			{ ShaderDataType::Float,  "a_tiling_mult" },
			{ ShaderDataType::Int,	  "a_entityID"},
			{ ShaderDataType::Int,    "a_Material_Index"}
			});
		s_Data.vao3D->AddVertexBuffer(s_Data.vertexBuffer3D);

		s_Data.cubeVertexBufferBase = new CubeVertex[s_Data.MaxVertices3D];

		uint32_t* cubeIndices = new uint32_t[s_Data.MaxIndices3D];

		offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices3D; i += 36) {
			cubeIndices[i + 0] = offset + 0;
			cubeIndices[i + 1] = offset + 1;
			cubeIndices[i + 2] = offset + 2;
			cubeIndices[i + 3] = offset + 3;
			cubeIndices[i + 4] = offset + 4;
			cubeIndices[i + 5] = offset + 5;

			cubeIndices[i + 6] = offset + 6;
			cubeIndices[i + 7] = offset + 7;
			cubeIndices[i + 8] = offset + 8;
			cubeIndices[i + 9] = offset + 9;
			cubeIndices[i + 10] = offset + 10;
			cubeIndices[i + 11] = offset + 11;

			cubeIndices[i + 12] = offset + 12;
			cubeIndices[i + 13] = offset + 13;
			cubeIndices[i + 14] = offset + 14;
			cubeIndices[i + 15] = offset + 15;
			cubeIndices[i + 16] = offset + 16;
			cubeIndices[i + 17] = offset + 17;

			cubeIndices[i + 18] = offset + 18;
			cubeIndices[i + 19] = offset + 19;
			cubeIndices[i + 20] = offset + 20;
			cubeIndices[i + 21] = offset + 21;
			cubeIndices[i + 22] = offset + 22;
			cubeIndices[i + 23] = offset + 23;

			cubeIndices[i + 24] = offset + 24;
			cubeIndices[i + 25] = offset + 25;
			cubeIndices[i + 26] = offset + 26;
			cubeIndices[i + 27] = offset + 27;
			cubeIndices[i + 28] = offset + 28;
			cubeIndices[i + 29] = offset + 29;

			cubeIndices[i + 30] = offset + 30;
			cubeIndices[i + 31] = offset + 31;
			cubeIndices[i + 32] = offset + 32;
			cubeIndices[i + 33] = offset + 33;
			cubeIndices[i + 34] = offset + 34;
			cubeIndices[i + 35] = offset + 35;

			offset += 36;
		}

		Ref<IndexBuffer> indexBuffer2 = IndexBuffer::Create(cubeIndices, s_Data.MaxIndices3D);
		s_Data.vao3D->SetIndexBuffer(indexBuffer2);
		delete[] cubeIndices;

		/// 3D /////////////////////////////////////////

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
		s_Data.tex_shader3D->bind();
		s_Data.tex_shader3D->setUniformIntArray("u_textures", samplers, s_Data.MaxTexSlots);

		s_Data.texture_slots[0] = s_Data.white_tex;

		s_Data.quadVertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.quadVertices[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.quadVertices[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.quadVertices[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		s_Data.texCoords[0] = { 0.0f, 0.0f };
		s_Data.texCoords[1] = { 1.0f, 0.0f };
		s_Data.texCoords[2] = { 1.0f, 1.0f };
		s_Data.texCoords[3] = { 0.0f, 1.0f };

		s_Data.cubeTexCoords[0] = { 0.0f, 0.0f };
		s_Data.cubeTexCoords[1] = { 1.0f, 0.0f };
		s_Data.cubeTexCoords[2] = { 1.0f, 1.0f };
		s_Data.cubeTexCoords[3] = { 1.0f, 1.0f };
		s_Data.cubeTexCoords[4] = { 0.0f, 1.0f };
		s_Data.cubeTexCoords[5] = { 0.0f, 0.0f };

		s_Data.camera_uniform_buffer3D = UniformBuffer::Create(sizeof(Renderer_Storeage::CameraData), 2);
		s_Data.materials_uniform_buffer3D = UniformBuffer::Create(sizeof(Renderer_Storeage::MaterialsData), 3);
		s_Data.plights_uniform_buffer3D = UniformBuffer::Create(sizeof(Renderer_Storeage::PointLightsData), 4);
		s_Data.dlight_uniform_buffer3D = UniformBuffer::Create(sizeof(Renderer_Storeage::DirLightData), 5);
		s_Data.camera_uniform_buffer = UniformBuffer::Create(sizeof(Renderer_Storeage::CameraData), 0);

	}

	void Renderer2D::Shutdown() {
		delete[] s_Data.quadVertexBufferBase;
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform, const uint32_t& plights) {
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

	void Renderer2D::BeginScene(EditorCamera& camera, const uint32_t& plights) {
		s_Data.tex_shader->bind();
		/*glm::mat4 viewproj = camera.getViewProjection();
		s_Data.tex_shader->setUniformMat4f("u_VP", viewproj);*/

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

	void Renderer2D::drawDirLight(const DirLightComponent& dlight) {
		s_Data.skyLight = dlight.dlight;
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
		Ref<Texture2D> skybox;
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
		{
			s_Data.cubeVertices[0] = { -0.5f, -0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[1] = { 0.5f, -0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[2] = { 0.5f,  0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[3] = { 0.5f,  0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[4] = { -0.5f,  0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[5] = { -0.5f, -0.5f, -0.5f,  1.0f };

			s_Data.cubeVertices[6] = { -0.5f, -0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[7] = { 0.5f, -0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[8] = { 0.5f,  0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[9] = { 0.5f,  0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[10] = { -0.5f,  0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[11] = { -0.5f, -0.5f,  0.5f,  1.0f };

			s_Data.cubeVertices[12] = { -0.5f,  0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[13] = { -0.5f,  0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[14] = { -0.5f, -0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[15] = { -0.5f, -0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[16] = { -0.5f, -0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[17] = { -0.5f,  0.5f,  0.5f,  1.0f };

			s_Data.cubeVertices[18] = { 0.5f,  0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[19] = { 0.5f,  0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[20] = { 0.5f, -0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[21] = { 0.5f, -0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[22] = { 0.5f, -0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[23] = { 0.5f,  0.5f,  0.5f,  1.0f };

			s_Data.cubeVertices[24] = { -0.5f, -0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[25] = { 0.5f, -0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[26] = { 0.5f, -0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[27] = { 0.5f, -0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[28] = { -0.5f, -0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[29] = { -0.5f, -0.5f, -0.5f,  1.0f };

			s_Data.cubeVertices[30] = { -0.5f,  0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[31] = { 0.5f,  0.5f, -0.5f,  1.0f };
			s_Data.cubeVertices[32] = { 0.5f,  0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[33] = { 0.5f,  0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[34] = { -0.5f,  0.5f,  0.5f,  1.0f };
			s_Data.cubeVertices[35] = { -0.5f,  0.5f, -0.5f,  1.0f };

			s_Data.cubeNormals[0] = { 0.0f, 0.0f, -1.0f };
			s_Data.cubeNormals[1] = { 0.0f, 0.0f, -1.0f };
			s_Data.cubeNormals[2] = { 0.0f, 0.0f, -1.0f };
			s_Data.cubeNormals[3] = { 0.0f, 0.0f, -1.0f };
			s_Data.cubeNormals[4] = { 0.0f, 0.0f, -1.0f };
			s_Data.cubeNormals[5] = { 0.0f, 0.0f, -1.0f };

			s_Data.cubeNormals[6] = { 0.0f, 0.0f, 1.0f };
			s_Data.cubeNormals[7] = { 0.0f, 0.0f, 1.0f };
			s_Data.cubeNormals[8] = { 0.0f, 0.0f, 1.0f };
			s_Data.cubeNormals[9] = { 0.0f, 0.0f, 1.0f };
			s_Data.cubeNormals[10] = { 0.0f, 0.0f, 1.0f };
			s_Data.cubeNormals[11] = { 0.0f, 0.0f, 1.0f };

			s_Data.cubeNormals[12] = { -1.0f, 0.0f, 0.0f };
			s_Data.cubeNormals[13] = { -1.0f, 0.0f, 0.0f };
			s_Data.cubeNormals[14] = { -1.0f, 0.0f, 0.0f };
			s_Data.cubeNormals[15] = { -1.0f, 0.0f, 0.0f };
			s_Data.cubeNormals[16] = { -1.0f, 0.0f, 0.0f };
			s_Data.cubeNormals[17] = { -1.0f, 0.0f, 0.0f };

			s_Data.cubeNormals[18] = { 1.0f, 0.0f, 0.0f };
			s_Data.cubeNormals[19] = { 1.0f, 0.0f, 0.0f };
			s_Data.cubeNormals[20] = { 1.0f, 0.0f, 0.0f };
			s_Data.cubeNormals[21] = { 1.0f, 0.0f, 0.0f };
			s_Data.cubeNormals[22] = { 1.0f, 0.0f, 0.0f };
			s_Data.cubeNormals[23] = { 1.0f, 0.0f, 0.0f };

			s_Data.cubeNormals[24] = { 0.0f, -1.0f, 0.0f };
			s_Data.cubeNormals[25] = { 0.0f, -1.0f, 0.0f };
			s_Data.cubeNormals[26] = { 0.0f, -1.0f, 0.0f };
			s_Data.cubeNormals[27] = { 0.0f, -1.0f, 0.0f };
			s_Data.cubeNormals[28] = { 0.0f, -1.0f, 0.0f };
			s_Data.cubeNormals[29] = { 0.0f, -1.0f, 0.0f };

			s_Data.cubeNormals[30] = { 0.0f, 1.0f, 0.0f };
			s_Data.cubeNormals[31] = { 0.0f, 1.0f, 0.0f };
			s_Data.cubeNormals[32] = { 0.0f, 1.0f, 0.0f };
			s_Data.cubeNormals[33] = { 0.0f, 1.0f, 0.0f };
			s_Data.cubeNormals[34] = { 0.0f, 1.0f, 0.0f };
			s_Data.cubeNormals[35] = { 0.0f, 1.0f, 0.0f };
		}

		std::vector<std::string> faces = {
			"Assets/Textures/skybox/right.jpg",
			"Assets/Textures/skybox/left.jpg",
			"Assets/Textures/skybox/top.jpg",
			"Assets/Textures/skybox/bottom.jpg",
			"Assets/Textures/skybox/front.jpg",
			"Assets/Textures/skybox/back.jpg",
		};
		s_cubemap.skybox = Texture2D::CreateCubemap(faces);

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

		s_cubemap.camera_uniform_buffer_skybox = UniformBuffer::Create(sizeof(CubeMap_Resources::CameraData_skybox), 1);
	}

	void Renderer3D::Shutdown3D() {
		delete[] s_Data.cubeVertexBufferBase;
	}

	void Renderer3D::BeginScene3D(const Camera& camera, const glm::mat4& transform) {
		s_Data.tex_shader3D->bind();

		s_Data.camera_buffer3D.view_projection3D = camera.getProjection() * glm::inverse(transform);
		s_Data.camera_buffer3D.camPos = glm::vec4(transform[3][0], transform[3][1], transform[3][2], 1.0f);
		s_Data.camera_uniform_buffer3D->setData(&s_Data.camera_buffer3D, sizeof(Renderer_Storeage::CameraData));
		s_Data.CubeIndCnt = 0;
		s_Data.cubeVertexBufferPtr = s_Data.cubeVertexBufferBase;

		s_Data.textureSlotInd = 1;
	}

	void Renderer3D::BeginScene3D(EditorCamera& camera) {
		s_Data.tex_shader3D->bind();

		s_Data.camera_buffer3D.view_projection3D = camera.getViewProjection();
		auto camPos = camera.getPosition();
		s_Data.camera_buffer3D.camPos = glm::vec4(camPos.x, camPos.y, camPos.z, 1.0f);
		s_Data.camera_uniform_buffer3D->setData(&s_Data.camera_buffer3D, sizeof(Renderer_Storeage::CameraData));
		s_Data.CubeIndCnt = 0;
		s_Data.cubeVertexBufferPtr = s_Data.cubeVertexBufferBase;

		s_Data.textureSlotInd = 1;
	}

	void Renderer3D::EndScene3D() {
		for (uint32_t i = 0; i < s_Data.textureSlotInd; i++) {
			s_Data.texture_slots[i]->bind(i);
		}

		for (size_t i = 0; i < MAX_MATERIALS; i++) {
			s_Data.materials_buffer3D.material[i] = s_Data.material[i];
		}

		for (size_t i = 0; i < s_Data.scene_plights; i++) {
			s_Data.plights_buffer3D.point_lights[i] = s_Data.point_lights[i];
		}
		s_Data.plights_buffer3D.nrLights = s_Data.scene_plights;

		s_Data.dlight_buffer3D.dlight = s_Data.skyLight;
		s_Data.dlight_uniform_buffer3D->setData(&s_Data.dlight_buffer3D, sizeof(Renderer_Storeage::DirLightData));
		s_Data.materials_uniform_buffer3D->setData(&s_Data.materials_buffer3D, sizeof(Renderer_Storeage::MaterialsData));
		s_Data.plights_uniform_buffer3D->setData(&s_Data.plights_buffer3D, sizeof(Renderer_Storeage::PointLightsData));

		Flush3D();
	}

	void Renderer3D::Flush3D() {
		uint32_t data_size = (uint32_t)((uint8_t*)s_Data.cubeVertexBufferPtr - (uint8_t*)s_Data.cubeVertexBufferBase);
		s_Data.vertexBuffer3D->SetData(s_Data.cubeVertexBufferBase, data_size);

		s_Data.tex_shader3D->bind();
		s_Data.vao3D->bind();
		RenderCommand::DrawIndexed(s_Data.vao3D, s_Data.CubeIndCnt);

		s_Data.stats.draw_calls_3d++;
		s_Data.FRAMES++;
	}

	void Renderer3D::Reset3D() {
		s_Data.CubeIndCnt = 0;
		s_Data.cubeVertexBufferPtr = s_Data.cubeVertexBufferBase;

		s_Data.textureSlotInd = 1;
	}

	void Renderer3D::drawCubeC(const glm::mat4& transform, const glm::vec4& color, int entityID) {
		if (s_Data.CubeIndCnt >= s_Data.MaxIndices3D) {
			EndScene3D();
			Reset3D();
		}

		const float textureInd = 0.0f; /// White texture
		const float tiling_mult = 1.0f;

		for (size_t i = 0; i < 36; i++) {
			s_Data.cubeVertexBufferPtr->position = transform * s_Data.cubeVertices[i];
			s_Data.cubeVertexBufferPtr->normal = s_Data.cubeNormals[i];
			s_Data.cubeVertexBufferPtr->color = color;
			s_Data.cubeVertexBufferPtr->tex_coord = s_Data.cubeTexCoords[i % 6];
			s_Data.cubeVertexBufferPtr->tex_index = textureInd;
			s_Data.cubeVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.cubeVertexBufferPtr->entityID = entityID;
			s_Data.cubeVertexBufferPtr++;
		}

		s_Data.CubeIndCnt += 36;

		s_Data.stats.cube_count++;
	}

	void Renderer3D::drawCubeCT(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& color, int entityID) {
		if (s_Data.CubeIndCnt >= s_Data.MaxIndices3D) {
			EndScene3D();
			Reset3D();
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

		const float tiling_mult = 1.0f;

		for (size_t i = 0; i < 36; i++) {
			s_Data.cubeVertexBufferPtr->position = transform * s_Data.cubeVertices[i];
			s_Data.cubeVertexBufferPtr->normal = s_Data.cubeNormals[i];
			s_Data.cubeVertexBufferPtr->color = color;
			s_Data.cubeVertexBufferPtr->tex_coord = s_Data.cubeTexCoords[i % 6];
			s_Data.cubeVertexBufferPtr->tex_index = textureInd;
			s_Data.cubeVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.cubeVertexBufferPtr->entityID = entityID;
			s_Data.cubeVertexBufferPtr++;
		}

		s_Data.CubeIndCnt += 36;

		s_Data.stats.cube_count++;
	}

	void Renderer3D::drawCubeM(const glm::mat4& transform, const glm::vec4& color, int material_index, int entityID) {
		if (s_Data.CubeIndCnt >= s_Data.MaxIndices3D) {
			EndScene3D();
			Reset3D();
		}

		const float textureInd = 0.0f; /// White texture
		const float tiling_mult = 1.0f;

		for (size_t i = 0; i < 36; i++) {
			s_Data.cubeVertexBufferPtr->position = transform * s_Data.cubeVertices[i];
			s_Data.cubeVertexBufferPtr->normal = transform * glm::vec4(s_Data.cubeNormals[i].x, s_Data.cubeNormals[i].y, s_Data.cubeNormals[i].z, 0.0f);
			s_Data.cubeVertexBufferPtr->color = color;
			s_Data.cubeVertexBufferPtr->tex_coord = s_Data.cubeTexCoords[i % 6];
			s_Data.cubeVertexBufferPtr->tex_index = textureInd;
			s_Data.cubeVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.cubeVertexBufferPtr->entityID = entityID;
			s_Data.cubeVertexBufferPtr->material_index = material_index;
			s_Data.cubeVertexBufferPtr++;

			//IARA_CORE_TRACE("Material Index: {0}", material_index);
		}

		s_Data.CubeIndCnt += 36;

		s_Data.stats.cube_count++;

	}

	void Renderer3D::addMaterial(const glm::vec4& ambient, const glm::vec4& diffuse, const glm::vec4& specular, const float& shininess) {
		Material mat;
		mat.ambient = ambient;
		mat.diffuse = diffuse;
		mat.specular = specular;
		mat.shininess = shininess;
		s_Data.material[s_Data.current_material_index++] = mat;

	}

	void Renderer3D::addMaterial(const uint32_t index, const glm::vec4& ambient, const glm::vec4& diffuse, const glm::vec4& specular, const float& shininess) {
		Material mat;
		mat.ambient = ambient;
		mat.diffuse = diffuse;
		mat.specular = specular;
		mat.shininess = shininess;
		s_Data.material[index] = mat;
	}

	void Renderer3D::drawSkyBox(const glm::mat4& view, const glm::mat4& projection) {
		RenderCommand::setDepthMask(false);
		s_cubemap.cubemap_shader->bind();

		//s_cubemap.cubemap_shader->setUniformInt("skybox", s_cubemap.skybox->getRendererID());
		s_cubemap.skybox->bind();
		s_cubemap.camera_buffer_skybox.view_projection3D = projection * view;
		s_cubemap.camera_uniform_buffer_skybox->setData(&s_cubemap.camera_buffer_skybox, sizeof(CubeMap_Resources::CameraData_skybox));

		s_cubemap.vao_cubemap->bind();
		RenderCommand::drawArrays(s_cubemap.vao_cubemap, 0, 36);
		RenderCommand::setDepthMask(true);
	}

	void Renderer3D::ResetStats3D() {
		memset(&s_Data.stats, 0, sizeof(Statistics));
	}

	Statistics Renderer3D::getStats3D() {
		return s_Data.stats;
	}

	uint32_t Renderer3D::getNrMaterials() {
		return s_Data.current_material_index;
	}

	Material Renderer3D::getMaterial(uint32_t index) {
		if (index < 10)
			return s_Data.material[index];
		return {};
	}

}
