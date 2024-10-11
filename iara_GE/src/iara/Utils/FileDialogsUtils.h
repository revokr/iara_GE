#pragma once

#include <string>

namespace iara {

	class FileDialogs {
	public:
		/// These return empty strings if canceled
		static std::string openFile(const char* filter);
		static std::string saveFile(const char* filter);
	private:

	};

}