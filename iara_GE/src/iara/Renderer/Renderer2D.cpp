#include "ir_pch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "shader.h"
#include "platform/openGL/OpenGLShader.h"
#include "RenderCommand.h"
#include <glm/ext/matrix_transform.hpp>

#define rad(x) glm::radians(x)

namespace iara {
	struct Renderer2D_Storeage {
		Ref<VertexArray> vao;
		Ref<Shader> tex_shader;
		Ref<Texture2D> white_tex;
	};

	static Renderer2D_Storeage* s_Data;

	void iara::Renderer2D::Init() {
		IARA_PROFILE_FUNCTION();

		s_Data = new Renderer2D_Storeage();

		s_Data->vao = (iara::VertexArray::Create());

		float vert[] = {
			-0.5f, -0.5f, -0.1f, 0.0f, 0.0f,
			 0.5f, -0.5f, -0.1f, 1.0f, 0.0f,
			-0.5f,  0.5f, -0.1f, 0.0f, 1.0f,
			 0.5f,  0.5f, -0.1f, 1.0f, 1.0f
		};

		iara::Ref<iara::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(iara::VertexBuffer::Create(vert, sizeof(vert)));

		iara::BufferLayout layout({
			{ iara::ShaderDataType::Float3, "a_pos" },
			{ iara::ShaderDataType::Float2, "a_tex" }
		});

		vertexBuffer->setLayout(layout);
		s_Data->vao->AddVertexBuffer(vertexBuffer);

		uint32_t indices[] = {
			0, 1, 2,
			1, 3, 2
		};

		iara::Ref<iara::IndexBuffer> indexBuffer;
		indexBuffer.reset(iara::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		s_Data->vao->SetIndexBuffer(indexBuffer);

		s_Data->white_tex = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data->white_tex->setData(&whiteTextureData, sizeof(uint32_t));

		s_Data->tex_shader = iara::Shader::Create("texture", "Shaders/texture.vert", "Shaders/texture.frag");
		s_Data->tex_shader->bind();
		s_Data->tex_shader->setUniformInt("u_texture", 0);
	}

	void iara::Renderer2D::Shutdown() {
		delete s_Data;
	}

	void iara::Renderer2D::BeginScene(const OrthographicCamera& camera) {
		s_Data->tex_shader->bind();
		s_Data->tex_shader->setUniformMat4f("u_VP", camera.getVP());
		s_Data->tex_shader->setUniformMat4f("u_transform", glm::mat4(1.0f));
	}

	void iara::Renderer2D::EndScene() {

	}

	void iara::Renderer2D::drawQuadC(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
		drawQuadC({ pos.x, pos.y, 0.0f }, size, color);
	}

	void iara::Renderer2D::drawQuadC(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color) {
		s_Data->tex_shader->setUniform4f("u_color", color);
		s_Data->white_tex->bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_Data->tex_shader->setUniformMat4f("u_transform", transform);

		s_Data->vao->bind();
		RenderCommand::DrawIndexed(s_Data->vao);
	}

	void Renderer2D::drawQuadT(const glm::vec2& pos, const glm::vec2& size, const Ref<Texture2D>& texture, float tiling_mult) {
		drawQuadT({ pos.x, pos.y, 0.0f }, size, texture, tiling_mult);
	}

	void Renderer2D::drawQuadT(const glm::vec3& pos, const glm::vec2& size, const Ref<Texture2D>& texture, float tiling_mult) {
		s_Data->tex_shader->setUniform4f("u_color", glm::vec4(1.0f));
		s_Data->tex_shader->setUniformFloat("u_tiling_mult", tiling_mult);
		texture->bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_Data->tex_shader->setUniformMat4f("u_transform", transform);

		s_Data->vao->bind();
		RenderCommand::DrawIndexed(s_Data->vao);
	}

	void Renderer2D::drawQuadTC(const glm::vec2& pos, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& color, float tiling_mult) {
		drawQuadTC({ pos.x, pos.y, 0.0f }, size, texture, color, tiling_mult);
	}

	void Renderer2D::drawQuadTC(const glm::vec3& pos, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& color, float tiling_mult) {
		s_Data->tex_shader->setUniform4f("u_color", color);
		s_Data->tex_shader->setUniformFloat("u_tiling_mult", tiling_mult);
		texture->bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_Data->tex_shader->setUniformMat4f("u_transform", transform);

		s_Data->vao->bind();
		RenderCommand::DrawIndexed(s_Data->vao);
	}

	void Renderer2D::drawQuadRC(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color) {
		drawQuadRC({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::drawQuadRC(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color) {
		s_Data->tex_shader->setUniform4f("u_color", color);
		s_Data->white_tex->bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_Data->tex_shader->setUniformMat4f("u_transform", transform);
		s_Data->vao->bind();
		RenderCommand::DrawIndexed(s_Data->vao);
	}

	void Renderer2D::drawQuadRT(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tiling_m) {
		drawQuadRT({ position.x, position.y, 0.0f }, size, rotation, texture, tiling_m);
	}

	void Renderer2D::drawQuadRT(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tiling_m) {
		s_Data->tex_shader->setUniform4f("u_color", glm::vec4(1.0f));
		s_Data->tex_shader->setUniformFloat("u_tiling_mult", tiling_m);
		texture->bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_Data->tex_shader->setUniformMat4f("u_transform", transform);

		s_Data->vao->bind();
		RenderCommand::DrawIndexed(s_Data->vao);
	}

	

}
