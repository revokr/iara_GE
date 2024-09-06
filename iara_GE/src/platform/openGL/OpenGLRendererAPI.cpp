#include "ir_pch.h"
#include "OpenGLRendererAPI.h"

#include "glad/glad.h"

namespace iara {
	void OpenGLRendererAPI::Init() {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glEnable(GL_DEPTH_TEST);
	}

	void OpenGLRendererAPI::setViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
		glViewport(x, y, width, height);
	}
	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& va) {
		glDrawElements(GL_TRIANGLES, va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
	}

}