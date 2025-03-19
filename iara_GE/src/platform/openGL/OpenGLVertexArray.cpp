#include "ir_pch.h"
#include "OpenGLVertexArray.h"
#include <glad/glad.h>
#include <vector>

namespace iara {

	static GLenum ShaderDataType_to_OpenGLBaseType(ShaderDataType type) {
		switch (type) {
		case ShaderDataType::Float:		return GL_FLOAT;
		case ShaderDataType::Float2:  	return GL_FLOAT;
		case ShaderDataType::Float3:	    return GL_FLOAT;
		case ShaderDataType::Float4:	    return GL_FLOAT;
		case ShaderDataType::Mat3:		return GL_FLOAT;
		case ShaderDataType::Mat4:		return GL_FLOAT;
		case ShaderDataType::Int:		    return GL_INT;
		case ShaderDataType::Int2:	    return GL_INT;
		case ShaderDataType::Int3:	    return GL_INT;
		case ShaderDataType::Int4:	    return GL_INT;
		case ShaderDataType::Bool:		return GL_BOOL;
		}

		IARA_CORE_ASSERT(false, "Unknown ShaderDataType.");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray() {
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray() {
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::bind() const {
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::unbind() const {
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::setVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) {
		IARA_CORE_ASSERT(vertexBuffer->getLayout().getElems().size(), "Vertex Buffer has no layout!");

		m_vertexbuffers.clear();

		glBindVertexArray(m_RendererID);
		vertexBuffer->bind();
		m_current_index = 0;
		const auto& layout = vertexBuffer->getLayout();
		for (const auto& el : layout) {
			switch (el.type)
			{
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			{
				glEnableVertexAttribArray(m_current_index);
				glVertexAttribPointer(m_current_index, el.getComponentCount(),
					ShaderDataType_to_OpenGLBaseType(el.type),
					el.normalized ? GL_TRUE : GL_FALSE,
					layout.getStride(), (const void*)el.offset);
				m_current_index++;
				break;
			}
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:
			case ShaderDataType::Bool:
			{
				glEnableVertexAttribArray(m_current_index);
				glVertexAttribIPointer(m_current_index, el.getComponentCount(),
					ShaderDataType_to_OpenGLBaseType(el.type),
					layout.getStride(), (const void*)el.offset);
				m_current_index++;
				break;
			}

			}
		}
		m_vertexbuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) {
		IARA_CORE_ASSERT(vertexBuffer->getLayout().getElems().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->bind();
		
		const auto& layout = vertexBuffer->getLayout();
		for (const auto& el : layout) {
			switch (el.type)
			{
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			{
				glEnableVertexAttribArray(m_current_index);
				glVertexAttribPointer(m_current_index, el.getComponentCount(),
					ShaderDataType_to_OpenGLBaseType(el.type),
					el.normalized ? GL_TRUE : GL_FALSE,
					layout.getStride(), (const void*)el.offset);
				m_current_index++;
				break;
			}
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:
			case ShaderDataType::Bool:
			{
				glEnableVertexAttribArray(m_current_index);
				glVertexAttribIPointer(m_current_index, el.getComponentCount(),
					ShaderDataType_to_OpenGLBaseType(el.type),
					layout.getStride(), (const void*)el.offset);
				m_current_index++;
				break;
			}

			}
		}
		m_vertexbuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) {
		glBindVertexArray(m_RendererID);
		indexBuffer->bind();

		m_indexbuffer = indexBuffer;
	}

	const Ref<IndexBuffer>& OpenGLVertexArray::getIndexBuffer() const {
		return m_indexbuffer;
	}

	const std::vector<Ref<VertexBuffer>>& OpenGLVertexArray::getVertexBuffers() const
	{
		return m_vertexbuffers;
	}

}
