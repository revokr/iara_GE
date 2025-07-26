#include "ir_pch.h"
#include "EditorCamera.h"

#include "iara\Core\Input.h"
#include "iara\Core\KeyCodes.h"

namespace iara {

    EditorCamera::EditorCamera(float fov, float aspect_ratio, float near_clip, float far_clip) 
        : m_fov{fov}, m_aspect_ratio{aspect_ratio}, m_near_clip{near_clip}, m_far_clip{far_clip},
        Camera{glm::perspective(glm::radians(fov), aspect_ratio, near_clip, far_clip)}
    {
        updateView();
    }

    void  EditorCamera::onUpdate(Timestep ts) {
        if (Input::IsKeyPressed(IARA_KEY_LEFT_ALT)) {
            const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
            glm::vec2 delta = (mouse - m_initial_mouse_pos) * 0.003f;
            m_initial_mouse_pos = mouse;

            if (Input::IsMouseButtonPressed(IARA_MOUSE_BUTTON_MIDDLE)) {
                mousePan(delta);
            }
            else if (Input::IsMouseButtonPressed(IARA_MOUSE_BUTTON_LEFT)) {
                mouseRotate(delta);
            }
            else if (Input::IsMouseButtonPressed(IARA_MOUSE_BUTTON_RIGHT)) {
                mouseZoom(delta.y);
            }
        }

        updateView();
    }

    void  EditorCamera::onEvent(Event& e) {
        EventDispatcher dispathcer(e);
        dispathcer.Dispatch<MouseScrolledEvent>(IARA_BIND_EVENT_FN(EditorCamera::onMouseScroll));
    }

    glm::vec3 EditorCamera::getUpDirection() const {
        return glm::rotate(getOrientation(), glm::vec3(0.0f, 1.0f, 0.0f)); 
    }

    glm::vec3 EditorCamera::getRightDirection() const {
        return glm::rotate(getOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 EditorCamera::getForwardDirection() const {
        return glm::rotate(getOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::quat EditorCamera::getOrientation() const {
        return glm::quat(glm::vec3(-m_pitch, -m_yaw, 0.0f));
    }

    void EditorCamera::updateProj() {
        m_aspect_ratio = m_viewport_w / m_viewport_h;
        m_projection = glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_near_clip, m_far_clip);
    }

    void EditorCamera::updateView() {
        /// m_yaw = 0.0f, m_pitch = 0.0f; Lock the camera's rotation
        m_position = calculatePosition();

        glm::quat orientation = getOrientation();
        m_view_matrix = glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(orientation);
        m_view_matrix = glm::inverse(m_view_matrix);
    }

    bool EditorCamera::onMouseScroll(MouseScrolledEvent& e){
        float delta = e.getOffsetY() * 0.1f;
        mouseZoom(delta);
        updateView();
        return false;
    }

    void EditorCamera::mousePan(const glm::vec2& delta) {
        auto [x_spd, y_spd] = panSpeed();
        m_focal_point += -getRightDirection() * delta.x * x_spd * m_distance;
        m_focal_point += getUpDirection() * delta.y * y_spd * m_distance;
    }

    void EditorCamera::mouseRotate(const glm::vec2& delta) {
        float yaw_sign = getUpDirection().y < 0 ? -1.0f : 1.0f;
        m_yaw += yaw_sign * delta.x * rotationSpeed();
        m_pitch += delta.y * rotationSpeed();
    }

    void EditorCamera::mouseZoom(float delta) {
        m_distance -= delta * zoomSpeed();
        if (m_distance < 1.0f) {
            m_focal_point += getForwardDirection();
            m_distance = 1.0f;  
        }
    }

    glm::vec3 EditorCamera::calculatePosition() const {
        return m_focal_point - getForwardDirection() * m_distance;
    }

    std::pair<float, float> EditorCamera::panSpeed() const {
        float x = std::min(m_viewport_w / 1000.0f, 2.4f); // max = 2.4f
        float x_factor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

        float y = std::min(m_viewport_h / 1000.0f, 2.4f);
        float y_factor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

        return { x_factor, y_factor };
    }

    float EditorCamera::rotationSpeed() const {
        return 0.8f;
    }

    float EditorCamera::zoomSpeed() const {
        float distance = m_distance * 0.2f;
        distance = std::max(distance, 0.0f);
        float speed = distance * distance;
        speed = std::min(speed, 100.0f);
        return speed;
    }

}
