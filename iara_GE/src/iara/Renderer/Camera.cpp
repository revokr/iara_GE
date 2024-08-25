#include "ir_pch.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>


iara::OrthographicCamera::OrthographicCamera(float left, float right, float bot, float top)
	: m_proj{ glm::ortho(left, right, bot, top, -1.0f, 1.0f) }, 
	  m_view{ 1.0f },
	  m_position{glm::vec3(0.0f, 0.0f, 0.0f)}
{
	m_VP = m_proj * m_view;
}

void iara::OrthographicCamera::recalculateView() {
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position);
	transform = glm::rotate(transform, glm::radians(m_rotation), glm::vec3(0, 0, 1));

	m_view = glm::inverse(transform);	
	m_VP = m_proj * m_view;
}
