#include "iara.h"

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public iara::Layer {
public:
	ExampleLayer() : Layer{ "Example" }, m_camera{-3.2f, 3.2f, -1.8f, 1.8f}, m_camerapos{0.0f, 0.0f, 5.5f},
		m_perspective_camera{1280.0f, 720.0f, 45.0f, 0.1f, 200.0f}
	{
		m_vertexArray.reset(iara::VertexArray::Create());

		float vert[7 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.4f, 0.2f, 0.6f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.3f, 0.2f, 0.3f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.3f, 0.6f, 0.6f, 1.0f,
			 0.5f,  0.5f, 0.0f, 0.8f, 0.4f, 0.2f, 1.0f
		};

		std::shared_ptr<iara::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(iara::VertexBuffer::Create(vert, sizeof(vert)));

		iara::BufferLayout layout({
			{ iara::ShaderDataType::Float3, "a_pos" },
			{ iara::ShaderDataType::Float4, "a_color" }
			});

		vertexBuffer->setLayout(layout);
		m_vertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[6] = {
			0, 1, 2,
			1, 3, 2
		};

		std::shared_ptr<iara::IndexBuffer> indexBuffer;
		indexBuffer.reset(iara::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		m_vertexArray->SetIndexBuffer(indexBuffer);

		m_vertexArrayTriangle.reset(iara::VertexArray::Create());
		float trianglevert[3 * 3] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.0f,  0.5f, 0.0f
		};

		std::shared_ptr<iara::VertexBuffer> trVertBuff;
		trVertBuff.reset(iara::VertexBuffer::Create(trianglevert, sizeof(trianglevert)));
		trVertBuff->setLayout({
			{iara::ShaderDataType::Float3, "a_pos"}
			});
		m_vertexArrayTriangle->AddVertexBuffer(trVertBuff);

		uint32_t indicesTriangle[3] = { 0, 1, 2 };
		std::shared_ptr<iara::IndexBuffer> trIndexBuff;
		trIndexBuff.reset(iara::IndexBuffer::Create(indicesTriangle, sizeof(indicesTriangle) / sizeof(uint32_t)));
		m_vertexArrayTriangle->SetIndexBuffer(trIndexBuff);

		m_shader.reset(new iara::Shader("Shaders/basic1.vert", "Shaders/basic1.frag"));
		m_shader2.reset(new iara::Shader("Shaders/basic2.vert", "Shaders/basic2.frag"));
		
	}

	void onUpdate(iara::Timestep ts) override {
		IARA_TRACE("Delta time : {0}s  ({1})ms" , ts.getSeconds(), ts.getMiliseconds());

		float time = ts;

		if (iara::Input::IsKeyPressed(IARA_KEY_A)) {
			m_camerapos.x -= m_camera_speed * time;
		}
		if (iara::Input::IsKeyPressed(IARA_KEY_D)) {
			m_camerapos.x += m_camera_speed * time;
		}
		if (iara::Input::IsKeyPressed(IARA_KEY_W)) {
			m_camerapos.y += m_camera_speed * time;
		}
		if (iara::Input::IsKeyPressed(IARA_KEY_S)) {
			m_camerapos.y -= m_camera_speed * time;
		}
		if (iara::Input::IsKeyPressed(IARA_KEY_R)) {
			m_camerapos.z += m_camera_speed * time;
		}
		if (iara::Input::IsKeyPressed(IARA_KEY_F)) {
			m_camerapos.z -= m_camera_speed * time;
		}

		if (iara::Input::IsKeyPressed(IARA_KEY_E)) {
			m_camera_rotation -= m_camera_rotation_speed * time;
		}
		if (iara::Input::IsKeyPressed(IARA_KEY_Q)) {
			m_camera_rotation += m_camera_rotation_speed * time;
		}

		iara::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.0f, 0.2f, 1.0f));
		iara::RenderCommand::Clear();

		m_camera.setPosition(m_camerapos);
		m_camera.setRotation(m_camera_rotation);

		m_perspective_camera.setPosition(m_camerapos);
		m_perspective_camera.setRotation(m_camera_rotation);

		iara::Renderer::BeginScene(m_perspective_camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));

		glm::vec4 redColor(0.8f, 0.2f, 0.3f, 1.0f);
		glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);

		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 20; j++) {
				glm::vec3 pos(i * 0.21f, j * 0.21f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				if ((i % 2 == 0 && j % 2 == 0) || (i % 2 == 1 && j % 2 == 1)) m_shader->setUniform4f("u_color", redColor);
				else m_shader->setUniform4f("u_color", blueColor);
				iara::Renderer::Submit(m_shader, m_vertexArray, transform);
			}
		}
		iara::Renderer::Submit(m_shader2, m_vertexArrayTriangle);
		iara::Renderer::EndScene();
	}

	void onEvent(iara::Event& event) override {

	}

	void onImGuiRender() override {
		
	}
private:
	std::shared_ptr<iara::VertexArray> m_vertexArray;
	std::shared_ptr<iara::Shader>	m_shader;

	std::shared_ptr<iara::VertexArray> m_vertexArrayTriangle;
	std::shared_ptr<iara::Shader>	m_shader2;

	iara::OrthographicCamera m_camera;
	iara::PerspectiveCamera m_perspective_camera;
	glm::vec3 m_camerapos;
	float m_camera_speed = 5.0f;
	float m_camera_rotation_speed = 100.0f;
	float m_camera_rotation = 0.0f;
};

class Sandbox : public iara::Application {
public:
	Sandbox() {
		pushLayer(new ExampleLayer());
	}

	~Sandbox() {}

};

iara::Application* CreateApplication() {
	return new Sandbox();
}