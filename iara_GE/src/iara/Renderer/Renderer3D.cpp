#include "ir_pch.h"
#include "Renderer3D.h"
#include "VertexArray.h"
#include "shader.h"
#include "platform/openGL/OpenGLShader.h"
#include "iara\Renderer\UniformBuffer.h"
#include "RenderCommand.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

//namespace iara {
//
//	struct CubeVertex {
//		glm::vec3 position;
//		glm::vec4 color;
//		glm::vec2 tex_coord;
//		float tex_index;
//		float tiling_mult;
//
//		/// Editor Only
//		int entityID;
//	};
//
//	struct Renderer3D_Storeage {
//		const uint32_t MaxQuads3D = 20000;
//		const uint32_t MaxVertices3D = MaxQuads3D * 4;
//		const uint32_t MaxIndices3D = MaxQuads3D * 36;
//		static const uint32_t MaxTexSlots3D = 32;
//
//		Ref<VertexArray> vao3D;
//		Ref<VertexBuffer> vertexBuffer3D;
//		Ref<Shader> tex_shader3D;
//		Ref<Texture2D> white_tex_3D;
//
//		uint32_t QuadIndCnt = 0;
//		CubeVertex* cubeVertexBufferBase = nullptr;
//		CubeVertex* cubeVertexBufferPtr = nullptr;
//
//		std::array<Ref<Texture2D>, MaxTexSlots3D> texture_slots;
//		uint32_t textureSlotInd = 1; /// 0 = white texture
//
//		glm::vec4 cubeVertices[8];
//		glm::vec2 texCoords[4];
//
//		Renderer3D::Statistics stats3D;
//
//		struct CameraData {
//			glm::mat4 view_projection;
//		};
//
//		CameraData camera_buffer;
//		Ref<UniformBuffer> camera_uniform_buffer;
//	};
//
//	static Renderer3D_Storeage s_Data3D;
//
//	void Renderer3D::Init3D() {
//		s_Data3D.vao3D = (VertexArray::Create());
//
//		s_Data3D.vertexBuffer3D = (VertexBuffer::Create(s_Data3D.MaxVertices3D * sizeof(CubeVertex)));
//
//		s_Data3D.vertexBuffer3D->setLayout({
//			{ ShaderDataType::Float3, "a_pos" },
//			{ ShaderDataType::Float4, "a_color" },
//			{ ShaderDataType::Float2, "a_tex" },
//			{ ShaderDataType::Float,  "a_tex_id" },
//			{ ShaderDataType::Float,  "a_tiling_mult" },
//			{ ShaderDataType::Int,	  "a_entityID"}
//			});
//		s_Data3D.vao3D->AddVertexBuffer(s_Data3D.vertexBuffer3D);
//
//		s_Data3D.cubeVertexBufferBase = new CubeVertex[s_Data3D.MaxVertices3D];
//
//		uint32_t* cubeIndices = new uint32_t[s_Data3D.MaxIndices3D];
//
//		uint32_t offset = 0;
//		for (uint32_t i = 0; i < s_Data3D.MaxIndices3D; i += 36) {
//			cubeIndices[i + 0] = offset + 0;
//			cubeIndices[i + 1] = offset + 1;
//			cubeIndices[i + 2] = offset + 2;
//			cubeIndices[i + 3] = offset + 2;
//			cubeIndices[i + 4] = offset + 3;
//			cubeIndices[i + 5] = offset + 0;
//
//			cubeIndices[i + 6] = offset + 4;
//			cubeIndices[i + 7] = offset + 5;
//			cubeIndices[i + 8] = offset + 6;
//			cubeIndices[i + 9]   = offset + 6;
//			cubeIndices[i + 10] = offset + 7;
//			cubeIndices[i + 11] = offset + 4;
//
//			cubeIndices[i + 12] = offset + 0;
//			cubeIndices[i + 13] = offset + 4;
//			cubeIndices[i + 14] = offset + 7;
//			cubeIndices[i + 15] = offset + 7;
//			cubeIndices[i + 16] = offset + 3;
//			cubeIndices[i + 17] = offset + 0;
//
//			cubeIndices[i + 18] = offset + 1;
//			cubeIndices[i + 19] = offset + 5;
//			cubeIndices[i + 20] = offset + 6;
//			cubeIndices[i + 21] = offset + 6;
//			cubeIndices[i + 22] = offset + 2;
//			cubeIndices[i + 23] = offset + 1;
//
//			cubeIndices[i + 24] = offset + 3;
//			cubeIndices[i + 25] = offset + 2;
//			cubeIndices[i + 26] = offset + 6;
//			cubeIndices[i + 27] = offset + 6;
//			cubeIndices[i + 28] = offset + 7;
//			cubeIndices[i + 29] = offset + 3;
//
//			cubeIndices[i + 30] = offset + 0;
//			cubeIndices[i + 31] = offset + 1;
//			cubeIndices[i + 32] = offset + 5;
//			cubeIndices[i + 33] = offset + 5;
//			cubeIndices[i + 34] = offset + 4;
//			cubeIndices[i + 35] = offset + 0;
//
//			offset += 8;
//		}
//
//		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(cubeIndices, s_Data3D.MaxIndices3D);
//		s_Data3D.vao3D->SetIndexBuffer(indexBuffer);
//		
//
//		s_Data3D.white_tex_3D = Texture2D::Create(1, 1);
//		uint32_t whiteTextureData = 0xffffffff;
//		s_Data3D.white_tex_3D->setData(&whiteTextureData, sizeof(uint32_t));
//
//		int samplers[s_Data3D.MaxTexSlots3D];
//		for (uint32_t i = 0; i < s_Data3D.MaxTexSlots3D; i++) {
//			samplers[i] = i;
//		}
//
//		s_Data3D.tex_shader3D = Shader::Create("texture3D", "Shaders/texture2.vert", "Shaders/texture2.frag");
//		s_Data3D.tex_shader3D->bind();
//		s_Data3D.tex_shader3D->setUniformIntArray("u_textures", samplers, s_Data3D.MaxTexSlots3D);
//
//		s_Data3D.texture_slots[0] = s_Data3D.white_tex_3D;
//
//		s_Data3D.cubeVertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
//		s_Data3D.cubeVertices[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
//		s_Data3D.cubeVertices[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
//		s_Data3D.cubeVertices[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
//		s_Data3D.cubeVertices[4] = { -0.5f, -0.5f, 1.0f, 1.0f };
//		s_Data3D.cubeVertices[5] = { 0.5f, -0.5f, 1.0f, 1.0f };
//		s_Data3D.cubeVertices[6] = { 0.5f,  0.5f, 1.0f, 1.0f };
//		s_Data3D.cubeVertices[7] = { -0.5f,  0.5f, 1.0f, 1.0f };
//
//		s_Data3D.texCoords[0] = { 0.0f, 0.0f };
//		s_Data3D.texCoords[1] = { 1.0f, 0.0f };
//		s_Data3D.texCoords[2] = { 1.0f, 1.0f };
//		s_Data3D.texCoords[3] = { 0.0f, 1.0f };
//
//		s_Data3D.camera_uniform_buffer = UniformBuffer::Create(sizeof(Renderer3D_Storeage::CameraData), 0);
//
//		delete[] cubeIndices;
//	}
//
//	void Renderer3D::Shutdown3D() {
//		delete[] s_Data3D.cubeVertexBufferBase;
//	}
//
//	void Renderer3D::BeginScene3D(const Camera& camera, const glm::mat4& transform)
//	{
//	}
//
//	void Renderer3D::BeginScene3D(EditorCamera& camera) {
//		s_Data3D.tex_shader3D->bind();
//
//		s_Data3D.camera_buffer.view_projection = camera.getViewProjection();
//		s_Data3D.camera_uniform_buffer->setData(&s_Data3D.camera_buffer, sizeof(Renderer3D_Storeage::CameraData));
//		s_Data3D.QuadIndCnt = 0;
//		s_Data3D.cubeVertexBufferPtr = s_Data3D.cubeVertexBufferBase;
//
//		s_Data3D.textureSlotInd = 1;
//	}
//
//	void Renderer3D::EndScene3D() {
//		for (uint32_t i = 0; i < s_Data3D.textureSlotInd; i++) {
//			s_Data3D.texture_slots[i]->bind(i);
//		}
//
//		Flush3D();
//	}
//
//	void Renderer3D::Flush3D() {
//		uint32_t data_size = (uint32_t)((uint8_t*)s_Data3D.cubeVertexBufferPtr - (uint8_t*)s_Data3D.cubeVertexBufferBase);
//		s_Data3D.vertexBuffer3D->SetData(s_Data3D.cubeVertexBufferBase, data_size);
//
//		s_Data3D.tex_shader3D->bind();
//
//		RenderCommand::DrawIndexed(s_Data3D.vao3D, s_Data3D.QuadIndCnt);
//
//		s_Data3D.stats3D.draw_calls_3d++;
//	}
//
//	void Renderer3D::Reset3D() {
//		s_Data3D.QuadIndCnt = 0;
//		s_Data3D.cubeVertexBufferPtr = s_Data3D.cubeVertexBufferBase;
//
//		s_Data3D.textureSlotInd = 1;
//	}
//
//	void Renderer3D::drawCubeC(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) { 
//		drawCubeC({ pos.x, pos.y, 0.0f }, size, color);
//	}
//
//	void Renderer3D::drawCubeC(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color) {
//		if (s_Data3D.QuadIndCnt >= s_Data3D.MaxIndices3D) {
//			EndScene3D();
//			Reset3D();
//		}
//
//		const float textureInd = 0.0f; /// White texture
//		const float tiling_mult = 1.0f;
//
//		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos)
//			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
//
//		for (size_t i = 0; i < 8; i++) {
//			s_Data3D.cubeVertexBufferPtr->position = transform * s_Data3D.cubeVertices[i];
//			s_Data3D.cubeVertexBufferPtr->color = color;
//			s_Data3D.cubeVertexBufferPtr->tex_coord = s_Data3D.texCoords[i % 4];
//			s_Data3D.cubeVertexBufferPtr->tex_index = textureInd;
//			s_Data3D.cubeVertexBufferPtr->tiling_mult = tiling_mult;
//			s_Data3D.cubeVertexBufferPtr++;
//		}
//
//		s_Data3D.QuadIndCnt += 36;
//
//		s_Data3D.stats3D.cube_count++;
//	}
//
//	void Renderer3D::drawCubeRC(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
//	{
//	}
//
//	void Renderer3D::drawCubeRC(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
//	{
//	}
//
//	void Renderer3D::drawCubeC(const glm::mat4& transform, const glm::vec4& color, int entityID) {
//		if (s_Data3D.QuadIndCnt >= s_Data3D.MaxIndices3D) {
//			EndScene3D();
//			Reset3D();
//		}
//
//		const float textureInd = 0.0f; /// White texture
//		const float tiling_mult = 1.0f;
//
//		for (size_t i = 0; i < 8; i++) {
//			s_Data3D.cubeVertexBufferPtr->position = transform * s_Data3D.cubeVertices[i];
//			s_Data3D.cubeVertexBufferPtr->color = color;
//			s_Data3D.cubeVertexBufferPtr->tex_coord = s_Data3D.texCoords[i % 4];
//			s_Data3D.cubeVertexBufferPtr->tex_index = textureInd;
//			s_Data3D.cubeVertexBufferPtr->tiling_mult = tiling_mult;
//			s_Data3D.cubeVertexBufferPtr->entityID = entityID;
//			s_Data3D.cubeVertexBufferPtr++;
//		}
//
//		s_Data3D.QuadIndCnt += 36;
//
//		s_Data3D.stats3D.cube_count++;
//	}
//
//	void Renderer3D::ResetStats3D() {
//		memset(&s_Data3D.stats3D, 0, sizeof(Statistics));
//	}
//
//	Renderer3D::Statistics Renderer3D::getStats3D() {
//		return s_Data3D.stats3D;
//	}
//
//}