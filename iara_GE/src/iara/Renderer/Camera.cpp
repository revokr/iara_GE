#include "ir_pch.h"
#include "Camera.h"

namespace iara {

	OrthographicCamera::OrthographicCamera(float left, float right, float bot, float top)
		: m_proj{ glm::ortho(left, right, bot, top, -1.0f, 1.0f) },
		m_view{ 1.0f },
		m_position{ glm::vec3(0.0f, 0.0f, 0.2f) }
	{
		m_VP = m_proj * m_view;
	}

	void OrthographicCamera::setProj(float left, float right, float bot, float top) {
		m_proj = glm::ortho(left, right, bot, top, -1.0f, 1.0f);
		m_VP = m_proj * m_view;
	}

	void OrthographicCamera::recalculateView() {
		IARA_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position);
		transform = glm::rotate(transform, glm::radians(m_rotation), glm::vec3(0, 0, 1));
		m_view = glm::inverse(transform);
		m_VP = m_proj * m_view;
	}

	PerspectiveCamera::PerspectiveCamera(float width, float height, float degrees, float close, float farr) :
		m_proj{ glm::perspective(glm::radians(degrees), width / height, close, farr) },
		m_view{ 1.0f },
		m_position{ glm::vec3(0.0f, 0.0f, 0.0f) }
	{
		m_VP = m_proj * m_view;
	}

	void PerspectiveCamera::setProj(float width, float height, float degrees, float close, float farr)
	{
		m_proj = glm::perspective(glm::radians(degrees), width / height, close, farr);
		m_VP = m_proj * m_view;
	}

	void PerspectiveCamera::recalculateView() {
		IARA_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position);
		transform = glm::rotate(transform, glm::radians(m_rotation), glm::vec3(0, 0, 1));

		m_view = glm::inverse(transform);
		m_VP = m_proj * m_view;
	}
}
