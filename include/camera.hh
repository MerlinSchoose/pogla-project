#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "shaders.hh"

enum Input_Keys {
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT,
    SHIFT
};

class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 up);

    void processKeyboard(Input_Keys key, bool press);
    void processMouse(float xoffset, float yoffset);
    void update_camera(std::vector<program *>& programs, float elapsed_time);

    [[nodiscard]] glm::mat4 view_matrix() const;
    [[nodiscard]] glm::mat4 projection_matrix() const;

private:
    void compute_vectors();
    void update_uniform(std::vector<program *>& programs) const;

    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    const glm::vec3 world_up_;
    glm::vec3 right_;

    static constexpr float movement_speed_ = 10.f;
    static constexpr float mouse_sensitivity_ = .1f;
    static constexpr float max_fov_ = 45.0f;

    float yaw_ = -90.0f;
    float pitch_ = 0.0f;
    float fov_camera_ = 45.0f;
    float sprint_speed_ = 10.0f;

    float delta_time_ = 0.f;
    float last_time_ = 0.f;

    struct key_being_pressed {
        bool forward;
        bool backward;
        bool left;
        bool right;
        bool shift;
    } key_pressed_;
};