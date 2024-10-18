#include "ir_pch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "shader.h"
#include "platform/openGL/OpenGLShader.h"
#include "iara\Renderer\UniformBuffer.h"
#include "RenderCommand.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#define rad(x) glm::radians(x)

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

	struct Renderer_Storeage {
		const uint32_t MaxQuads = 20000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTexSlots = 32;

		const uint32_t MaxQuads3D = 20000;
		const uint32_t MaxVertices3D = MaxQuads3D * 4;
		const uint32_t MaxIndices3D = MaxQuads3D * 36;

		Ref<VertexArray> vao;
		Ref<VertexArray> vao3D;
		Ref<VertexBuffer> vertexBuffer;
		Ref<VertexBuffer> vertexBuffer3D;
		Ref<Shader> tex_shader;
		Ref<Texture2D> white_tex;

		uint32_t QuadIndCnt = 0;
		uint32_t CubeIndCnt = 0;
		QuadVertex* quadVertexBufferBase = nullptr;
		QuadVertex* quadVertexBufferPtr = nullptr;

		QuadVertex* cubeVertexBufferBase = nullptr;
		QuadVertex* cubeVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTexSlots> texture_slots;
		uint32_t textureSlotInd = 1; /// 0 = white texture

		glm::vec4 quadVertices[4];
		glm::vec4 cubeVertices[8];
		glm::vec2 texCoords[4];

	    Statistics stats;

		struct CameraData {
			glm::mat4 view_projection3D;
		};

		CameraData camera_buffer;
		Ref<UniformBuffer> camera_uniform_buffer;

		CameraData camera_buffer3D;
		Ref<UniformBuffer> camera_uniform_buffer3D;
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

		s_Data.vertexBuffer3D = (VertexBuffer::Create(s_Data.MaxVertices3D * sizeof(QuadVertex)));

		s_Data.vertexBuffer3D->setLayout({
			{ ShaderDataType::Float3, "a_pos" },
			{ ShaderDataType::Float4, "a_color" },
			{ ShaderDataType::Float2, "a_tex" },
			{ ShaderDataType::Float,  "a_tex_id" },
			{ ShaderDataType::Float,  "a_tiling_mult" },
			{ ShaderDataType::Int,	  "a_entityID"}
			});
		s_Data.vao3D->AddVertexBuffer(s_Data.vertexBuffer3D);

		s_Data.cubeVertexBufferBase = new QuadVertex[s_Data.MaxVertices3D];

		uint32_t* cubeIndices = new uint32_t[s_Data.MaxIndices3D];

		offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices3D; i += 36) {
			cubeIndices[i + 0] = offset + 0;
			cubeIndices[i + 1] = offset + 1;
			cubeIndices[i + 2] = offset + 2;
			cubeIndices[i + 3] = offset + 2;
			cubeIndices[i + 4] = offset + 3;
			cubeIndices[i + 5] = offset + 0;

			cubeIndices[i + 6] = offset + 4;
			cubeIndices[i + 7] = offset + 5;
			cubeIndices[i + 8] = offset + 6;
			cubeIndices[i + 9] = offset + 6;
			cubeIndices[i + 10] = offset + 7;
			cubeIndices[i + 11] = offset + 4;

			cubeIndices[i + 12] = offset + 0;
			cubeIndices[i + 13] = offset + 4;
			cubeIndices[i + 14] = offset + 7;
			cubeIndices[i + 15] = offset + 7;
			cubeIndices[i + 16] = offset + 3;
			cubeIndices[i + 17] = offset + 0;

			cubeIndices[i + 18] = offset + 1;
			cubeIndices[i + 19] = offset + 5;
			cubeIndices[i + 20] = offset + 6;
			cubeIndices[i + 21] = offset + 6;
			cubeIndices[i + 22] = offset + 2;
			cubeIndices[i + 23] = offset + 1;

			cubeIndices[i + 24] = offset + 3;
			cubeIndices[i + 25] = offset + 2;
			cubeIndices[i + 26] = offset + 6;
			cubeIndices[i + 27] = offset + 6;
			cubeIndices[i + 28] = offset + 7;
			cubeIndices[i + 29] = offset + 3;

			cubeIndices[i + 30] = offset + 0;
			cubeIndices[i + 31] = offset + 1;
			cubeIndices[i + 32] = offset + 5;
			cubeIndices[i + 33] = offset + 5;
			cubeIndices[i + 34] = offset + 4;
			cubeIndices[i + 35] = offset + 0;

			offset += 8;
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

		s_Data.texture_slots[0] = s_Data.white_tex;

		s_Data.quadVertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.quadVertices[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.quadVertices[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.quadVertices[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		s_Data.texCoords[0] = { 0.0f, 0.0f };
		s_Data.texCoords[1] = { 1.0f, 0.0f };
		s_Data.texCoords[2] = { 1.0f, 1.0f };
		s_Data.texCoords[3] = { 0.0f, 1.0f };

		s_Data.camera_uniform_buffer3D = UniformBuffer::Create(sizeof(Renderer_Storeage::CameraData), 0);
		s_Data.camera_uniform_buffer = UniformBuffer::Create(sizeof(Renderer_Storeage::CameraData), 0);

	}

	void Renderer2D::Shutdown() {
		delete[] s_Data.quadVertexBufferBase;
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform) {
		glm::mat4 viewproj = camera.getProjection() * glm::inverse(transform);

		/*s_Data.tex_shader->bind();
		s_Data.tex_shader->setUniformMat4f("u_VP", viewproj);*/

		s_Data.camera_buffer.view_projection3D = camera.getProjection() * glm::inverse(transform);
		s_Data.camera_uniform_buffer->setData(&s_Data.camera_buffer, sizeof(Renderer_Storeage::CameraData));
		s_Data.QuadIndCnt = 0;
		s_Data.quadVertexBufferPtr = s_Data.quadVertexBufferBase;

		s_Data.textureSlotInd = 1;
	}

	void Renderer2D::BeginScene(EditorCamera& camera) {
		s_Data.tex_shader->bind();
		/*glm::mat4 viewproj = camera.getViewProjection();
		s_Data.tex_shader->setUniformMat4f("u_VP", viewproj);*/

		s_Data.camera_buffer.view_projection3D = camera.getViewProjection();
		s_Data.camera_uniform_buffer->setData(&s_Data.camera_buffer, sizeof(Renderer_Storeage::CameraData));
		s_Data.QuadIndCnt = 0;
		s_Data.quadVertexBufferPtr = s_Data.quadVertexBufferBase;

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
		drawQuadT(transform, texture, {1.0f, 1.0f, 1.0f, 1.0f}, tiling_mult);
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
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f})
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
		} else {
			drawQuadC(transform, src.color, entityID);
		}
	}

	void Renderer2D::ResetStats() {
		memset(&s_Data.stats, 0, sizeof(Statistics));
	}

	Statistics Renderer2D::getStats(){
		return s_Data.stats;
	}
}

namespace iara {

	struct CubeVertex {
		glm::vec3 position;
		glm::vec4 color;
		glm::vec2 tex_coord;
		float tex_index;
		float tiling_mult;

		/// Editor Only
		int entityID;
	};

	void Renderer3D::Init3D() {
		s_Data.cubeVertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.cubeVertices[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.cubeVertices[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.cubeVertices[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.cubeVertices[4] = { -0.5f, -0.5f, 1.0f, 1.0f };
		s_Data.cubeVertices[5] = { 0.5f, -0.5f, 1.0f, 1.0f };
		s_Data.cubeVertices[6] = { 0.5f,  0.5f, 1.0f, 1.0f };
		s_Data.cubeVertices[7] = { -0.5f,  0.5f, 1.0f, 1.0f };
	}

	void Renderer3D::Shutdown3D() {
		delete[] s_Data.cubeVertexBufferBase;
	}

	void Renderer3D::BeginScene3D(const Camera& camera, const glm::mat4& transform)
	{
	}

	void Renderer3D::BeginScene3D(EditorCamera& camera) {
		s_Data.tex_shader->bind();

		s_Data.camera_buffer3D.view_projection3D = camera.getViewProjection();
		s_Data.camera_uniform_buffer3D->setData(&s_Data.camera_buffer3D, sizeof(Renderer_Storeage::CameraData));
		s_Data.CubeIndCnt = 0;
		s_Data.cubeVertexBufferPtr = s_Data.cubeVertexBufferBase;

		s_Data.textureSlotInd = 1;
	}

	void Renderer3D::EndScene3D() {
		for (uint32_t i = 0; i < s_Data.textureSlotInd; i++) {
			s_Data.texture_slots[i]->bind(i);
		}

		Flush3D();
	}

	void Renderer3D::Flush3D() {
		uint32_t data_size = (uint32_t)((uint8_t*)s_Data.cubeVertexBufferPtr - (uint8_t*)s_Data.cubeVertexBufferBase);
		s_Data.vertexBuffer3D->SetData(s_Data.cubeVertexBufferBase, data_size);

		s_Data.tex_shader;
		s_Data.vao3D->bind();
		RenderCommand::DrawIndexed(s_Data.vao3D, s_Data.CubeIndCnt);

		s_Data.stats.draw_calls_3d++;
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

		for (size_t i = 0; i < 8; i++) {
			s_Data.cubeVertexBufferPtr->position = transform * s_Data.cubeVertices[i];
			s_Data.cubeVertexBufferPtr->color = color;
			s_Data.cubeVertexBufferPtr->tex_coord = s_Data.texCoords[i % 4];
			s_Data.cubeVertexBufferPtr->tex_index = textureInd;
			s_Data.cubeVertexBufferPtr->tiling_mult = tiling_mult;
			s_Data.cubeVertexBufferPtr->entityID = entityID;
			s_Data.cubeVertexBufferPtr++;
		}

		s_Data.CubeIndCnt += 36;

		s_Data.stats.cube_count++;
	}

	void Renderer3D::ResetStats3D() {
		memset(&s_Data.stats, 0, sizeof(Statistics));
	}

	Statistics Renderer3D::getStats3D() {
		return s_Data.stats;
	}

}
