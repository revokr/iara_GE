#pragma once

#include "iara/Core/Log.h"
#include "Debug/Instrumentor.h"

#include <memory>
#include <functional>
#include <utility>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>


#ifdef IARA_PLATFORM_WINDOWS
#include "Windows.h"
#endif