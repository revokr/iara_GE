#pragma once

#include "iara\Renderer\Camera.h"

namespace iara {

	class SceneCamera : public Camera {
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1};
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void  setOrthographic(float size, float nearClip, float farClip);
		void  setPerspective(float vertical_fov, float nearClip, float farClip);

		void  setViewportSize(uint32_t width, uint32_t height);

		float getPerspectiveNearClip() const { return m_perspective_near; }
		void  setPerspectiveNearClip(float near_clip) { m_perspective_near = near_clip; recalculateProj(); }
		float getPerspectiveFarClip() const { return m_perspective_far; }
		void  setPerspectiveFarClip(float far_clip) { m_perspective_far = far_clip; recalculateProj(); }
		float getPerspectiveVerticalFov() const { return m_perspective_fov; }
		void  setPerspectiveVerticalFov(float fov) { m_perspective_fov = fov; recalculateProj(); }

		float getOrthoNearClip() const { return m_ortho_near; }
		void  setOrthoNearClip(float near_clip) { m_ortho_near = near_clip; recalculateProj();}
		float getOrthoFarClip() const { return m_ortho_far; }
		void  setOrthoFarClip(float far_clip) { m_ortho_far = far_clip; recalculateProj();}
		float getOrthographicSize() const { return m_ortho_size; }
		void  setOrthographicSize(float size) { m_ortho_size = size; recalculateProj(); }

		void  setProjectionType(ProjectionType type) { m_proj_type = type; recalculateProj(); }
		ProjectionType getProjectionType() const { return m_proj_type; }
	private:
		void recalculateProj();
	private:
		ProjectionType m_proj_type = ProjectionType::Orthographic;

		float m_ortho_size = 10.0f;
		float m_ortho_near = -1.0f, m_ortho_far = 1.0f;

		float m_perspective_fov = 45.0f;
		float m_perspective_near = 0.001f, m_perspective_far = 1000.0f;

		float m_aspect_ratio = 0.0f;
	};

}