#pragma once

#include <glm/glm.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT
};

class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 up);

    void processKeyboard(Camera_Movement direction, float deltaTime);
    void processMouse(float xoffset, float yoffset);
    void processScroll(float yoffset);
    glm::mat4 view_matrix() const;
    void shift_pressed(bool is_pressed);

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    const glm::vec3 worldUp;
    glm::vec3 right;

    static constexpr float movement_speed = 10.f;
    static constexpr float mouse_sensitivity = .1f;
    static constexpr float max_fov = 45.0f;

    float yaw = -90.0f;
    float pitch = 0.0f;
    float fov_camera = 45.0f;
    bool shiftPressed = false;
    float sprintSpeedUp = 10.0f;

    void refresh_vectors();
};