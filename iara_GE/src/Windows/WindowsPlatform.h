#pragma once

#include "Platform.h"

namespace iara {
	class WindowsPlatform : public Platform {
	public:
		virtual float getTimeImpl() override;
	};

}