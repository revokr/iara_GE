#pragma once
#include <cstdint>

namespace iara {

	enum class ShaderDataType {
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type) {
		switch (type) {
			case ShaderDataType::Float:		return 4;
			case ShaderDataType::Float2:	return 2 * 4;
			case ShaderDataType::Float3:	return 3 * 4;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Mat3:		return 4 * 3 * 3;
			case ShaderDataType::Mat4:		return 4 * 4 * 4;
			case ShaderDataType::Int:		return 4;
			case ShaderDataType::Int2:	    return 2 * 4;
			case ShaderDataType::Int3:	    return 3 * 4;
			case ShaderDataType::Int4:	    return 4 * 4;
			case ShaderDataType::Bool:		return 1;
		}

		IARA_CORE_ASSERT(false, "Unknown ShaderDataType.");
		return 0;
	}

	struct BufferElem {
		std::string name;	
		ShaderDataType type;
		uint32_t offset;
		uint32_t size;
		bool normalized;

		BufferElem() {}

		BufferElem(ShaderDataType type_arg, const std::string& name_arg)
			: name{ name_arg }, type{ type_arg },
			  size{ ShaderDataTypeSize(type) }, offset{ 0 }, normalized{ false } 
		{
		}

		uint32_t getComponentCount() const {
			switch (type) {
				case ShaderDataType::Float:		return 1;
				case ShaderDataType::Float2:	return 2;
				case ShaderDataType::Float3:	return 3;
				case ShaderDataType::Float4:	return 4;
				case ShaderDataType::Mat3:		return 3 * 3;
				case ShaderDataType::Mat4:		return 4 * 4;
				case ShaderDataType::Int:		return 1;
				case ShaderDataType::Int2:	    return 2;
				case ShaderDataType::Int3:	    return 3;
				case ShaderDataType::Int4:	    return 4;
				case ShaderDataType::Bool:		return 1;
			}

			IARA_CORE_ASSERT(false, "Unknown ShaderDataType.");
			return 0;
		}
	};

	class BufferLayout {
	public:
		BufferLayout() {}

		BufferLayout(std::initializer_list<BufferElem> elements) 
			: m_Elems{ elements } {
			CalculateOffsetAndStride();
		}

		inline const std::vector<BufferElem> getElems() const { return m_Elems; }
		inline const uint32_t getStride() const { return m_stride; }

		std::vector<BufferElem>::iterator begin() { return m_Elems.begin(); }
		std::vector<BufferElem>::iterator end() { return m_Elems.end(); }
		std::vector<BufferElem>::const_iterator begin() const { return m_Elems.begin(); }
		std::vector<BufferElem>::const_iterator end() const { return m_Elems.end(); }
	private:
		void CalculateOffsetAndStride() {
			uint32_t offset = 0;
			for (auto& el : m_Elems) {
				el.offset = offset;
				offset += el.size;
				m_stride += el.size;
			}
		}
	private:
		std::vector<BufferElem> m_Elems;
		uint32_t m_stride = 0;
	};

	class VertexBuffer {
	public:
		virtual ~VertexBuffer() {}

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual const BufferLayout& getLayout() const = 0;
		virtual void setLayout(const BufferLayout& layout) = 0;

		static VertexBuffer* Create(float* vertices, uint32_t size);
	};

	class IndexBuffer {
	public:
		virtual ~IndexBuffer() {}

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual uint32_t getCount() const = 0;

		static IndexBuffer* Create(uint32_t* indices, uint32_t size);
	};

}