#pragma once

namespace iara {

	class Platform {
	public:

		inline static float getTime() { return s_Instance->getTimeImpl(); }

	protected:
		virtual float getTimeImpl() = 0;
	private:
		static Platform* s_Instance;
	};

}