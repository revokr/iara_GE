#include "ir_pch.h"
#include "CameraController.h"
#include "iara/Core/Input.h"
#include "iara/Core/KeyCodes.h"

namespace iara {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_aspectRatio{aspectRatio},
		m_camera{-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel},
		m_rotation{rotation}
	{

	}

	void OrthographicCameraController::onUpdate(Timestep ts) {
		float time = ts;

		if (Input::IsKeyPressed(IARA_KEY_A)) {
			m_camerapos.x -= m_camera_speed * time;
		}
		if (Input::IsKeyPressed(IARA_KEY_D)) {
			m_camerapos.x += m_camera_speed * time;
		}
		if (Input::IsKeyPressed(IARA_KEY_W)) {
			m_camerapos.y += m_camera_speed * time;
		}
		if (Input::IsKeyPressed(IARA_KEY_S)) {
			m_camerapos.y -= m_camera_speed * time;
		}

		if (m_rotation) {
			if (Input::IsKeyPressed(IARA_KEY_E)) {
				m_camera_rotation -= m_camera_rotation_speed * time;
			}
			if (Input::IsKeyPressed(IARA_KEY_Q)) {
				m_camera_rotation += m_camera_rotation_speed * time;
			}

			m_camera.setRotation(m_camera_rotation);
		}

		m_camera.setPosition(m_camerapos);

		m_camera_speed = m_zoomLevel;
	}

	void OrthographicCameraController::onEvent(Event& e) {
		EventDispatcher dispather(e);
		dispather.Dispatch<MouseScrolledEvent>(IARA_BIND_EVENT_FN(OrthographicCameraController::onMouseScroll));
		dispather.Dispatch<WindowResizeEvent>(IARA_BIND_EVENT_FN(OrthographicCameraController::onWindowResize));
	}

	bool OrthographicCameraController::onMouseScroll(MouseScrolledEvent& e)	{
		if (e.getOffsetY() < 0) m_zoomLevel -= -0.15f;
		else m_zoomLevel -= 0.15f;
		m_camera.setProj(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);
		return false;
	}

	bool OrthographicCameraController::onWindowResize(WindowResizeEvent& e) {
		m_aspectRatio = ((float)e.getWidth() / (float)e.getHeight());
		m_camera.setProj(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);
		return false;
	}

	PerspectiveCameraController::PerspectiveCameraController(float width, float height, float degrees, float close, float farr)
		: m_camera{width, height, degrees, close, farr}, m_aspectRatio{width / height},
		m_degrees{degrees}, m_zClose{close}, m_zFar{farr}, m_width{width}, m_height{height}
	{
	}

	void PerspectiveCameraController::onUpdate(Timestep ts) {
		IARA_PROFILE_FUNCTION();

		float time = ts;

		if (Input::IsKeyPressed(IARA_KEY_A)) {
			m_camerapos.x -= m_camera_speed * time;
		}
		if (Input::IsKeyPressed(IARA_KEY_D)) {
			m_camerapos.x += m_camera_speed * time;
		}
		if (Input::IsKeyPressed(IARA_KEY_W)) {
			m_camerapos.y += m_camera_speed * time;
		}
		if (Input::IsKeyPressed(IARA_KEY_S)) {
			m_camerapos.y -= m_camera_speed * time;
		}
		if (Input::IsKeyPressed(IARA_KEY_R)) {
			m_camerapos.z += m_camera_speed * time;
		}
		if (Input::IsKeyPressed(IARA_KEY_F)) {
			m_camerapos.z -= m_camera_speed * time;
		}

		if (Input::IsKeyPressed(IARA_KEY_E)) {
			m_camera_rotation -= m_camera_rotation_speed * time;
		}
		if (Input::IsKeyPressed(IARA_KEY_Q)) {
			m_camera_rotation += m_camera_rotation_speed * time;
		}
		m_camera.setRotation(m_camera_rotation);

		m_camera.setPosition(m_camerapos);
	}

	void PerspectiveCameraController::onEvent(Event& e) {
		EventDispatcher dispather(e);
		dispather.Dispatch<MouseScrolledEvent>(IARA_BIND_EVENT_FN(PerspectiveCameraController::onMouseScroll));
		dispather.Dispatch<WindowResizeEvent>(IARA_BIND_EVENT_FN(PerspectiveCameraController::onWindowResize));
	}

	bool PerspectiveCameraController::onMouseScroll(MouseScrolledEvent& e)
	{
		if (e.getOffsetY() < 0) m_zoomLevel -= -0.15f;
		else m_zoomLevel -= 0.15f;
		m_camera.setProj(m_width * m_zoomLevel, m_height * m_zoomLevel, (m_degrees * m_zoomLevel), m_zClose, m_zFar);
		return false;
	}

	bool PerspectiveCameraController::onWindowResize(WindowResizeEvent& e)
	{
		m_width = (float)e.getWidth();
		m_height = (float)e.getHeight();
		if (m_width != 0 && m_height != 0)
			m_camera.setProj(m_width * m_zoomLevel, m_height * m_zoomLevel, (m_degrees * m_zoomLevel), m_zClose, m_zFar);
		return false;
	}

}