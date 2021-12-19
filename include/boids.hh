#pragma once

#include "objects.hh"

class Boid : public Object {
public:
    Boid(const std::vector<Object> &objects,
         const std::vector<Vao *> &vaos,
         glm::vec3 min_pos, glm::vec3 max_pos,
         float min_scale, float max_scale,
         float min_rot, float max_rot,
         glm::vec3 vec_rot, float size,
         float min_speed, float max_speed)
            : Object(objects, vaos,
                     min_pos, max_pos,
                     min_scale, max_scale,
                     min_rot, max_rot,
                     vec_rot, size) {
        float speed = ((float) rand() / (float) RAND_MAX) * (max_speed - min_speed) + min_speed;
        velocity_ = glm::vec3(sinf(rot_), 0.f, cosf(rot_)) * speed;

        acceleration_ = glm::vec3(0.f);

        prev_pos_ = pos_;
        prev_velocity_ = velocity_;
    }

    void move(const std::vector<Boid>& boids, float elapsed_time);

private:
    std::vector<Boid> get_neighbours(const std::vector<Boid>& boids);

    glm::vec3 align_force(const std::vector<Boid>& neighbours);
    glm::vec3 cohesion_force(const std::vector<Boid>& neighbours);
    glm::vec3 separation_force(const std::vector<Boid>& neighbours);

    glm::vec3 velocity_;
    glm::vec3 acceleration_;

    glm::vec3 prev_pos_;
    glm::vec3 prev_velocity_;

    float min_speed_ = 1.f;
    float max_speed_ = 3.f;
    float max_steer_ = .5f;

    float perception_distance_ = 10.f;

    float align_weight_ = 1.f;
    float cohesion_weight_ = 1.f;
    float separation_weight_ = 1.f;

    float delta_time_ = 0.f;
    float last_time_ = 0.f;
};