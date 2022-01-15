#include <glm/ext/matrix_transform.hpp>

#include "camera.hh"

Camera::Camera(glm::vec3 position, glm::vec3 up)
        : position_(position)
        , up_(up)
        , world_up_(up) {
    compute_vectors();
}

void Camera::compute_vectors() {
    float x_dir = cosf(glm::radians(yaw_)) * cosf(glm::radians(pitch_));
    float y_dir = sinf(glm::radians(pitch_));
    float z_dir = sinf(glm::radians(yaw_)) * cosf(glm::radians(pitch_));
    front_ = glm::normalize(glm::vec3(x_dir, y_dir, z_dir));

    right_ = glm::normalize(glm::cross(front_, world_up_));
    up_ = glm::normalize(glm::cross(right_, front_));
}

void Camera::processKeyboard(Input_Keys key, bool press) {
    switch (key) {
        case Input_Keys::FORWARD:
            key_pressed_.forward = press;
            break;
        case Input_Keys::BACKWARD:
            key_pressed_.backward = press;
            break;
        case Input_Keys::LEFT:
            key_pressed_.left = press;
            break;
        case Input_Keys::RIGHT:
            key_pressed_.right = press;
            break;
        case Input_Keys::SHIFT:
            key_pressed_.shift = press;
            break;
    }
}

void Camera::processMouse(float xoffset, float yoffset) {
    xoffset *= mouse_sensitivity_;
    yoffset *= mouse_sensitivity_;

    yaw_ += xoffset;
    pitch_ += yoffset;

    float max_pitch = 89.0f;
    if (pitch_ > max_pitch)
        pitch_ = max_pitch;
    if (pitch_ < -max_pitch)
        pitch_ = -max_pitch;

    compute_vectors();
}

void Camera::update_uniform(std::vector<program *>& programs) const {
    for (auto& program: programs) {
        program->use();
        GLint mv_loc = glGetUniformLocation(program->id, "view_matrix");TEST_OPENGL_ERROR();
        glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &view_matrix()[0][0]);TEST_OPENGL_ERROR();

        GLint cp_loc = glGetUniformLocation(program->id, "cameraPos");TEST_OPENGL_ERROR();
        glUniform3f(cp_loc, position_.x, position_.y, position_.z);TEST_OPENGL_ERROR();
    }
}

void Camera::update_camera(std::vector<program *>& programs, float elapsed_time) {
    update_uniform(programs);

    delta_time_ = elapsed_time - last_time_;
    last_time_ = elapsed_time;

    float cameraSpeed = movement_speed_ * (delta_time_ / 1000.f);
    if (key_pressed_.shift)
        cameraSpeed *= sprint_speed_;

    if (key_pressed_.forward)
        position_ += front_ * cameraSpeed;
    if (key_pressed_.backward)
        position_ -= front_ * cameraSpeed;
    if (key_pressed_.left)
        position_ -= right_ * cameraSpeed;
    if (key_pressed_.right)
        position_ += right_ * cameraSpeed;
}

glm::mat4 Camera::view_matrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
}

glm::mat4 Camera::projection_matrix() const {
    return glm::perspective(
            fov_camera_,
            16.f/9.f,
            1.f, 2500.0f
    );
}