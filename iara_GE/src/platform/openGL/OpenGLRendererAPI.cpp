 #include "ir_pch.h"
#include "OpenGLRendererAPI.h"

#include "glad/glad.h"

namespace iara {
	void OpenGLRendererAPI::Init() {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
	}

	void OpenGLRendererAPI::setViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
		glViewport(x, y, width, height);
	}
	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearDepth(GL_LESS);
	}

	void OpenGLRendererAPI::setDepthMask(bool set) {
		if (set == true)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}

	void OpenGLRendererAPI::polygonMode(bool enable) {
		if (enable) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	void OpenGLRendererAPI::BindTextureUnit(uint32_t slot, uint32_t tex) {
		glBindTextureUnit(slot, tex);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& va, uint32_t indexCount) {
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::DrawIndexedBaseVertex(const Ref<VertexArray>& va, uint32_t indexCount, uint32_t indexStart, uint32_t vertexStart) {
		glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * indexStart), vertexStart);
	}

	void OpenGLRendererAPI::drawArray(const Ref<VertexArray>& va, uint32_t start, uint32_t end) {
		glDrawArrays(GL_TRIANGLES, start, end);
	}
}