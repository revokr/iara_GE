#include "ir_pch.h"
#include "SceneCamera.h"

namespace iara {

	SceneCamera::SceneCamera() {
		recalculateProj();
	}

	void SceneCamera::setOrthographic(float size, float nearClip, float farClip) {
		m_ortho_size = size;
		m_ortho_near = nearClip;
		m_ortho_far  = farClip;
		recalculateProj();
	}

	void SceneCamera::setPerspective(float vertical_fov, float nearClip, float farClip) {
		m_proj_type = ProjectionType::Perspective;
		m_perspective_fov = vertical_fov;
		m_perspective_near = nearClip;
		m_perspective_far = farClip;
		recalculateProj();
	}

	void SceneCamera::setViewportSize(uint32_t width, uint32_t height) {
		m_aspect_ratio = (float)width / (float)height;

		recalculateProj();
	}

	void SceneCamera::recalculateProj() {
		if (m_proj_type == SceneCamera::ProjectionType::Perspective)
		{
			m_projection = glm::perspective(glm::radians(m_perspective_fov), m_aspect_ratio, m_perspective_near, m_perspective_far);
		}
		else
		{
			float ortho_left = -0.5f * m_aspect_ratio * m_ortho_size;
			float ortho_right = 0.5f * m_aspect_ratio * m_ortho_size;
			float ortho_bot = -0.5f * m_ortho_size;
			float ortho_top = 0.5f * m_ortho_size;

			m_projection = glm::ortho(ortho_left, ortho_right, ortho_bot, ortho_top, m_ortho_near, m_ortho_far);
		}
	}
}
