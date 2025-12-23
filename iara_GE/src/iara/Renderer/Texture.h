#pragma once
#include <cstdint>

namespace iara {

	class Texture {
	public:
		virtual ~Texture() = default;

		virtual void bind(uint32_t slot = 0) const = 0;
		virtual void unbind() const = 0;

		virtual void setData(void* data, uint32_t size) = 0;

		virtual const uint32_t getWidth() const = 0;
		virtual const uint32_t getHeight() const = 0;
		virtual const uint32_t getRendererID() const = 0;
		virtual const std::string getPath() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	private:

	};

	class Texture2D : public Texture {
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::string& path);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, uint32_t internal_format, uint32_t format, uint32_t type, void* data);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, uint32_t internal_format, uint32_t format);
		static Ref<Texture2D> CreateCubemap(const std::string& faces);
	};

	class Texture3D : public Texture {
		public:
		static Ref<Texture3D> Create(uint32_t width, uint32_t height, uint32_t depth);
		static Ref<Texture3D> Create(const std::string& path);
		static Ref<Texture3D> Create(uint32_t width, uint32_t height, uint32_t depth, uint32_t internal_format, uint32_t format);
	};

}