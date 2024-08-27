#pragma once
#include <cstdint>

namespace iara {

	class Texture {
	public:
		virtual ~Texture() = default;

		virtual void bind(uint32_t slot = 0) const = 0;
		virtual void unbind() const = 0;

		virtual const uint32_t getWidth() const = 0;
		virtual const uint32_t getHeight() const = 0;
	private:

	};

	class Texture2D : public Texture {
	public:
		static Ref<Texture2D> Create(const std::string& path);
	};

}