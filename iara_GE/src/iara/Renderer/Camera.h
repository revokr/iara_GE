#pragma once

#include <glm/glm.hpp>

namespace iara {

	class OrthographicCamera {
	public:
		//OrthographicCamera() {}
		OrthographicCamera(float left, float right, float bot, float top);

		float getRotation() const { return m_rotation; }
		void setRotation(float rot) { m_rotation = rot; recalculateView(); }

		const glm::vec3& getPosition() const { return m_position; }
		void setPosition(const glm::vec3& pos) { m_position = pos; recalculateView(); }

		void setProj(float left, float right, float bot, float top);
			
		const glm::mat4& getProj() const { return m_proj; }
		const glm::mat4& getView() const { return m_view; }
		const glm::mat4& getVP()   const { return m_VP; }
	private:
		void recalculateView();
	private:
		glm::mat4 m_proj;
		glm::mat4 m_view;
		glm::mat4 m_VP;

		glm::vec3 m_position;
		float m_rotation = 0.0f;
	};

	class PerspectiveCamera {
	public:
		PerspectiveCamera(float width, float height, float degrees, float close, float farr);

		float getRotation() const { return m_rotation; }
		void setRotation(float rot) { m_rotation = rot; recalculateView(); }

		const glm::vec3& getPosition() const { return m_position; }
		void setPosition(const glm::vec3& pos) { m_position = pos; recalculateView(); }

		void setProj(float width, float height, float degrees, float close, float farr);

		const glm::mat4& getProj() const { return m_proj; }
		const glm::mat4& getView() const { return m_view; }
		const glm::mat4& getVP()   const { return m_VP; }
	private:
		void recalculateView();
	private:
		glm::mat4 m_proj;
		glm::mat4 m_view;
		glm::mat4 m_VP;

		glm::vec3 m_position;
		float m_rotation = 0.0f;
	};

}