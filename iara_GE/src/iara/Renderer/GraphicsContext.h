#pragma once

namespace iara {

	class GraphicsContext {
	public:
		virtual void Init()        = 0;
		virtual void SwapBuffers() = 0;
	private:

	};

}