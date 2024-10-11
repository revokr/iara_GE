#pragma once

#include <glm\glm.hpp>

namespace iara {

	namespace math {

		bool decomposeTransform(const glm::mat4& transform, glm::vec3& out_translation, glm::vec3& rotation, glm::vec3& scale);

	}

}