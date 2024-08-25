#include "iara.h"

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public iara::Layer {
public:
	ExampleLayer() : Layer{ "Example" }, m_camera{-3.2f, 3.2f, -1.8f, 1.8f}, m_camerapos{0.0f, 0.0f, 0.0f}
	
	{
		m_vertexArray.reset(iara::VertexArray::Create());

		float vert[7 * 4] = {
			-0.75f, -0.75f, 0.0f, 0.4f, 0.2f, 0.6f, 1.0f,
			 0.75f, -0.75f, 0.0f, 0.3f, 0.2f, 0.3f, 1.0f,
			-0.75f,  0.75f, 0.0f, 0.3f, 0.6f, 0.6f, 1.0f,
			 0.75f,  0.75f, 0.0f, 0.8f, 0.4f, 0.2f, 1.0f
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

		std::string vertSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_pos;
			layout(location = 1) in vec4 a_color;		

			out vec3 v_pos;
			out vec4 v_color;
			uniform mat4 u_MVP;

			void main() {
				v_pos = a_pos;
				v_color = a_color;
				gl_Position = u_MVP * vec4(a_pos, 1.0);
			}		

		)";

		std::string fragSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_pos;
			in vec4 v_color;

			void main() {
				color = v_color;
			}		

		)";

		std::string vertSrc2 = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_pos;

			out vec3 v_pos;
			uniform mat4 u_MVP;

			void main() {
				v_pos = a_pos;
				gl_Position = u_MVP * vec4(a_pos, 1.0);
			}		

		)";

		std::string fragSrc2 = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_pos;

			void main() {
				color = vec4(1.0, 0.4, 0.5, 1.0);
			}		

		)";

		m_shader.reset(new iara::Shader(vertSrc, fragSrc));
		m_shader2.reset(new iara::Shader(vertSrc2, fragSrc2));
		
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

		iara::Renderer::BeginScene(m_camera);

		iara::Renderer::Submit(m_shader, m_vertexArray);
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