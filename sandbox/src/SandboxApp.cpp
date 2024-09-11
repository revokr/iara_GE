#include "iara.h"

// -----Entry Point-----------------------
#include "iara/Core/EntryPoint.h"
// ---------------------------------------

#include "imgui/imgui.h"
#include <memory>
#include "platform/openGL/OpenGLShader.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Game_Test/GameLayer.h"
#include "Sandbox2D.h"

class ExampleLayer : public Layer {
public:
	ExampleLayer() : Layer{ "Example" }, m_camera_ctrl{1280.0f / 720.0f, true}, m_camerapos{0.0f, 0.0f, 5.5f},
		m_perspective_camera_ctrl{1280.0f, 720.0f, 45.0f, 0.01f, 2000.0f},
		m_customcolor{0.0f} 
	{
		FramebufferSpecification fb_spec;
		fb_spec.width = 300;
		fb_spec.height = 150;
		m_fb = Framebuffer::Create(fb_spec);
		m_vertexArray = (VertexArray::Create());

		float vert[] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f
		};

		Ref<VertexBuffer> vertexBuffer;
		vertexBuffer = (VertexBuffer::Create(vert, sizeof(vert)));

		BufferLayout layout({
			{ ShaderDataType::Float3, "a_pos" }
			});

		vertexBuffer->setLayout(layout);
		m_vertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[] = {
			0, 1, 2,
			1, 3, 2
		};

		Ref<IndexBuffer> indexBuffer;
		indexBuffer = (IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		m_vertexArray->SetIndexBuffer(indexBuffer);

		m_vertexArraySquare = (VertexArray::Create());
		float trianglevert[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f
		};

		Ref<VertexBuffer> trVertBuff;
		trVertBuff = (VertexBuffer::Create(trianglevert, sizeof(trianglevert)));
		trVertBuff->setLayout({
			{ShaderDataType::Float3, "a_pos"},
		    { ShaderDataType::Float2, "a_tex" }
			});
		m_vertexArraySquare->AddVertexBuffer(trVertBuff);

		uint32_t indicesTriangle[6] = { 
			0, 1, 2,
			1, 3, 2
		};
		Ref<IndexBuffer> trIndexBuff;
		trIndexBuff = (IndexBuffer::Create(indicesTriangle, sizeof(indicesTriangle) / sizeof(uint32_t)));
		m_vertexArraySquare->SetIndexBuffer(trIndexBuff);


		m_shader = Shader::Create("basic1", "Shaders/basic1.vert", "Shaders/basic1.frag");
		m_shader2 = (Shader::Create("basic2", "Shaders/basic2.vert", "Shaders/basic2.frag"));

		auto shader_tex = m_shaderLibrary.load("texture", "Shaders/texture.vert", "Shaders/texture.frag");


		m_texture = Texture2D::Create("Assets/Textures/horse.png");
		m_checkers = Texture2D::Create("Assets/Textures/check.png");

		std::dynamic_pointer_cast<OpenGLShader>(shader_tex)->bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader_tex)->setUniformInt("u_texture", 0);
	}

	void onUpdate(Timestep ts) override {
		//IARA_TRACE("Delta time : {0}s  ({1})ms" , ts.getSeconds(), ts.getMiliseconds());

		// Update
		m_camera_ctrl.onUpdate(ts);
		m_perspective_camera_ctrl.onUpdate(ts);

		// Render
		float time = ts;

		RenderCommand::SetClearColor(glm::vec4(0.1f, 0.0f, 0.2f, 1.0f));
		RenderCommand::Clear();

		Renderer::BeginScene(m_perspective_camera_ctrl.getCamera());

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));

		glm::vec4 redColor(0.8f, 0.2f, 0.3f, 1.0f);
		glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);


		auto shader_tex = m_shaderLibrary.get("texture");
		shader_tex->bind();
		shader_tex->setUniform4f("u_color", color::White);
		m_checkers->bind();
		glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(1.9f))
			* glm::translate(glm::mat4(1.0), glm::vec3(0.30f, 0.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		Renderer::Submit(shader_tex, m_vertexArraySquare, transform);

		m_shader->bind();
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 20; j++) {
				glm::vec3 pos(i * 0.23f, j * 0.231f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				if ((i % 2 == 0 && j % 2 == 0) || (i % 2 == 1 && j % 2 == 1)) 
					m_shader->setUniform4f("u_color", redColor);
				else 
					m_shader->setUniform4f("u_color", blueColor);
				Renderer::Submit(m_shader, m_vertexArray, transform);
			}
		}
		
		m_texture->bind();
		Renderer::Submit(shader_tex, m_vertexArraySquare, glm::scale(glm::mat4(1.0f), glm::vec3(1.9f))
			* glm::translate(glm::mat4(1.0), glm::vec3(0.7f, 0.0f, 0.1f)));

		Renderer::EndScene();
		
	}

	void onEvent(Event& event) override {
		IARA_TRACE(event.ToString());
		m_camera_ctrl.onEvent(event);
		m_perspective_camera_ctrl.onEvent(event);
	}

	void onImGuiRender() override {
		ImGui::Begin("asd");
		ImGui::SliderFloat4("color", &m_customcolor.x, 0.0f, 1.0f);
		ImGui::End();
	}
private:
	ShaderLibrary m_shaderLibrary;
	Ref<VertexArray> m_vertexArray;
	Ref<Shader>	m_shader;

	Ref<VertexArray> m_vertexArraySquare;
	Ref<Shader>	m_shader2;

	Ref<Texture2D> m_texture, m_checkers;
	Ref<Framebuffer> m_fb;

	OrthographicCameraController m_camera_ctrl;
	PerspectiveCameraController m_perspective_camera_ctrl;
	glm::vec3 m_camerapos;
	float m_camera_speed = 5.0f;
	float m_camera_rotation_speed = 100.0f;
	float m_camera_rotation = 0.0f;

	glm::vec4 m_customcolor;
};

class Sandbox : public iara::Application {
public:
	Sandbox() {
		//pushLayer(new ExampleLayer());
		//pushLayer(new GameLayer());
		pushLayer(new Sandbox2D());
	}

	~Sandbox() {}

};

iara::Application* CreateApplication() {
	return new Sandbox();
}