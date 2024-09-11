#pragma once

#include "Camera.h"
#include "iara/Core/Timestep.h"
#include "iara/events/AppEvent.h"
#include "iara/events/MouseEvent.h"

namespace iara {

	struct OrthographicCameraBounds {
		float left, right, bot, top;

		float getWidth() { return right - left; }
		float getHeight() { return top - bot; }
	};

	class OrthographicCameraController {
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false); // 

		void onUpdate(Timestep ts);
		void onEvent(Event& e);

		OrthographicCamera& getCamera() { return m_camera; }
		const OrthographicCamera& getCamera() const { return m_camera; }
		const OrthographicCameraBounds& getBounds() const { return m_bounds; }

	private:
		bool onMouseScroll(MouseScrolledEvent& e);
		bool onWindowResize(WindowResizeEvent& e);
	private:
		float m_aspectRatio;
		float m_zoomLevel = 3.0f;
		OrthographicCameraBounds m_bounds;
		OrthographicCamera m_camera;

		bool m_rotation;
		glm::vec3 m_camerapos = { 0.0f, 0.0f, -0.3f };
		float m_camera_rotation = 0.0f;
		
		float m_camera_speed = 1.0f, m_camera_rotation_speed = 100.0f;

	};

	class PerspectiveCameraController {
	public:
		PerspectiveCameraController(float width, float height, float degrees, float close, float farr); // 

		void onUpdate(Timestep ts);
		void onEvent(Event& e);

		PerspectiveCamera& getCamera() { return m_camera; }
		const PerspectiveCamera& getCamera() const { return m_camera; }

	private:
		bool onMouseScroll(MouseScrolledEvent& e);
		bool onWindowResize(WindowResizeEvent& e);
	private:
		float m_aspectRatio, m_degrees, m_zClose, m_zFar, m_width, m_height;
		float m_zoomLevel = 1.0f;
		PerspectiveCamera m_camera;

		glm::vec3 m_camerapos = { 0.0f, 0.0f, 4.0f };
		float m_camera_rotation = 0.0f;

		float m_camera_speed = 2.0f, m_camera_rotation_speed = 1.0f;

	};

}