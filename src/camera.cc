#include <glm/ext/matrix_transform.hpp>

#include "camera.hh"

#include <iostream>

Camera::Camera(glm::vec3 position, glm::vec3 up)
        : position(position)
        , up(up)
        , worldUp(up)
{
    refresh_vectors();
}

void Camera::processKeyboard(Camera_Movement direction, float deltaTime) {
    float cameraSpeed = movement_speed * deltaTime;
    if (shiftPressed)
        cameraSpeed *= sprintSpeedUp;
    switch (direction) {
        case Camera_Movement::FORWARD:
            position += front * cameraSpeed;
            break;
        case Camera_Movement::BACKWARD:
            position -= front * cameraSpeed;
            break;
        case Camera_Movement::LEFT:
            position -= right * cameraSpeed;
            break;
        case Camera_Movement::RIGHT:
            position += right * cameraSpeed;
            break;
    }
}

void Camera::processMouse(float xoffset, float yoffset) {
    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    float max_pitch = 89.0f;
    if (pitch > max_pitch)
        pitch = max_pitch;
    if (pitch < -max_pitch)
        pitch = -max_pitch;

    refresh_vectors();
}

void Camera::processScroll(float yoffset) {
    fov_camera -= yoffset;
    if (fov_camera < 1.0f)
        fov_camera = 1.0f;
    if (fov_camera > max_fov)
        fov_camera = max_fov;
}

glm::mat4 Camera::view_matrix() const {
    return glm::lookAt(position, position + front, up);
}

void Camera::refresh_vectors() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(direction);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));

}

void Camera::shift_pressed(bool is_pressed) {
    shiftPressed = is_pressed;
}