#pragma once

#include "objects.hh"

class Boid : public Object {
public:
    Boid(const std::vector<Object> &objects,
         const std::vector<Vao *> &vaos,
         glm::vec3 min_pos, glm::vec3 max_pos,
         float min_rot_x, float max_rot_x,
         float min_rot_y, float max_rot_y,
         float min_rot_z, float max_rot_z,
         float min_speed, float max_speed,
         float min_scale, float max_scale, float size)
            : Object(objects, vaos,
                     min_pos, max_pos,
                     min_rot_x, max_rot_x,
                     min_rot_y, max_rot_y,
                     min_rot_z, max_rot_z,
                     min_scale, max_scale, size) {
        float speed = ((float) rand() / (float) RAND_MAX) * (max_speed - min_speed) + min_speed;

        velocity_ = glm::vec3(rot_mat_ * glm::vec4(0.f, 0.f, 1.f, 1.f) * speed);

        acceleration_ = glm::vec3(0.f);

        prev_pos_ = pos_;
        prev_velocity_ = velocity_;
    }

    void move(const std::vector<Boid>& boids, const std::vector<Object>& objects, float elapsed_time);

private:
    std::vector<Boid> get_neighbours(const std::vector<Boid>& boids);

    glm::vec3 align_force(const std::vector<Boid>& neighbours);
    glm::vec3 cohesion_force(const std::vector<Boid>& neighbours);
    glm::vec3 separation_force(const std::vector<Boid>& neighbours);
    glm::vec3 avoid_objects(const std::vector<Object>& objects);
    glm::vec3 restrict_boundaries();

    glm::vec3 velocity_;
    glm::vec3 acceleration_;

    glm::vec3 prev_pos_;
    glm::vec3 prev_velocity_;

    float min_speed_ = 2.f;
    float max_speed_ = 5.f;
    float max_steer_ = 2.f;

    float perception_distance_ = 10.f;

    float align_weight_ = 1.f;
    float cohesion_weight_ = 1.f;
    float separation_weight_ = 1.f;
    float avoid_weight_ = 1.f;
    float restrict_weight_ = 2.f;

    float delta_time_ = 0.f;
    float last_time_ = 0.f;
};