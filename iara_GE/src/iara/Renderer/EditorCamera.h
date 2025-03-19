#pragma once

#include "Camera.h"
#include "iara\events\MouseEvent.h"
#include "iara\Core\Timestep.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm\gtx\quaternion.hpp>

namespace iara {

	class EditorCamera : public Camera {
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspect_ratio, float near_clip, float far_clip);

		void onUpdate(Timestep ts);
		void onEvent(Event& e);

		inline float getDistance() const { return m_distance; }
		inline void setDistance(float dist) { m_distance = dist; }

		inline void setViewportSize(float width, float height) { m_viewport_w = width; m_viewport_h = height; updateProj(); }

		const glm::mat4& getViewMatrix() const { return m_view_matrix; }
		glm::mat4 getViewProjection() { return m_projection * m_view_matrix; }

		glm::vec3 getUpDirection() const;
		glm::vec3 getRightDirection() const;
		glm::vec3 getForwardDirection() const;
		const glm::vec3& getPosition() const { return m_position; }
		glm::quat getOrientation() const;

		float getPitch() const { return m_pitch; }
		float getYaw() const { return m_yaw; }
	private:
		void updateProj();
		void updateView();

		bool onMouseScroll(MouseScrolledEvent& e);

		void mousePan(const glm::vec2& delta);
		void mouseRotate(const glm::vec2& delta);
		void mouseZoom(float delta);

		glm::vec3 calculatePosition() const;

		std::pair<float, float> panSpeed() const;
		float rotationSpeed() const;
		float zoomSpeed() const;
	private:
		float m_fov = 60.0f, m_aspect_ratio = 1.778f, m_near_clip = 0.1f, m_far_clip = 1000.0f;

		glm::mat4 m_view_matrix;
		glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_focal_point = { 0.0f,0.0f, 0.0f };

		glm::vec2 m_initial_mouse_pos;

		float m_distance = 200.0f;
		float m_pitch = 0.0f, m_yaw = 0.0f;

		float m_viewport_w = 1280, m_viewport_h = 720;

	};

}