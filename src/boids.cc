#include <algorithm>

#include "boids.hh"

std::vector<Boid> Boid::get_neighbours(const std::vector<Boid>& boids) {
    std::vector<Boid> neighbours;
    for (auto& boid : boids) {
        if (boid.pos_ != pos_ && glm::distance(boid.pos_, pos_) < perception_distance_)
            neighbours.emplace_back(boid);
    }

    return neighbours;
}

glm::vec3 Boid::align_force(const std::vector<Boid>& neighbours) {
    glm::vec3 avg_dir = velocity_;
    for (auto& neighbour: neighbours)
        avg_dir += neighbour.prev_velocity_;

    glm::vec3 align_force = -velocity_;
    avg_dir = avg_dir / (float) (neighbours.size() + 1);

    if (glm::length(avg_dir) > 0)
        align_force += glm::normalize(avg_dir);

    // std::cout << "Align: (" << align_force.x << ", " << align_force.y << ", " << align_force.z << ")\n";

    if (glm::length(align_force) > max_steer_)
        return glm::normalize(align_force) * max_steer_;

    return align_force;
}

glm::vec3 Boid::cohesion_force(const std::vector<Boid>& neighbours) {
    glm::vec3 center = pos_;
    for (auto& neighbour: neighbours)
        center += neighbour.prev_pos_;

    glm::vec3 cohesion_force(0.f);
    if (!neighbours.empty()) {
        center = center / (float) (neighbours.size() + 1);

        if (glm::length(center - pos_) > 0)
            cohesion_force += glm::normalize(center - pos_);

        // std::cout << "Cohesion: (" << cohesion_force.x << ", " << cohesion_force.y << ", " << cohesion_force.z << ")\n";

        if (glm::length(cohesion_force) > max_steer_)
            return glm::normalize(cohesion_force) * max_steer_;
    }

    return cohesion_force;
}

glm::vec3 Boid::separation_force(const std::vector<Boid>& neighbours) {
    glm::vec3 avg_dir(0.f);
    for (auto& neighbour: neighbours)
        avg_dir += ((pos_ - neighbour.prev_pos_) / glm::distance(pos_, neighbour.prev_pos_));

    glm::vec3 separation_force(0.f);
    if (!neighbours.empty()) {
        avg_dir = avg_dir / (float) neighbours.size();

        if (glm::length(avg_dir) > 0)
            separation_force += glm::normalize(avg_dir);

        // std::cout << "Separation: (" << separation_force.x << ", " << separation_force.y << ", " << separation_force.z << ")\n";

        if (glm::length(separation_force) > max_steer_)
            return glm::normalize(separation_force) * max_steer_;
    }

    return separation_force;
}

void Boid::move(const std::vector<Boid>& boids, float elapsed_time) {
    delta_time_ = (elapsed_time - last_time_) / 1000.f;
    last_time_ = elapsed_time;

    prev_pos_ = pos_;
    prev_velocity_ = velocity_;

    // std::cout << "Delta Time:" << delta_time_ << std::endl;
    // std::cout << "Position: (" << pos_.x << ", " << pos_.y << ", " << pos_.z << ")\n";
    // std::cout << "Velocity: (" << velocity_.x << ", " << velocity_.y << ", " << velocity_.z << ")\n";

    auto neighbours = get_neighbours(boids);
    // std::cout << "Neighbours: " << neighbours.size() << std::endl;
    acceleration_ += align_force(neighbours) * align_weight_;
    acceleration_ += cohesion_force(neighbours) * cohesion_weight_;
    acceleration_ += separation_force(neighbours) * separation_weight_;

    velocity_ += acceleration_ * delta_time_;

    float speed = glm::length(velocity_);
    // std::cout << "Acceleration: (" << acceleration_.x << ", " << acceleration_.y << ", " << acceleration_.z << ")\n";
    // std::cout << "Updated Velocity: (" << velocity_.x << ", " << velocity_.y << ", " << velocity_.z << ")\n";
    // std::cout << "Speed: " << speed << std::endl;
    glm::vec3 dir = velocity_ / speed;
    speed = std::clamp(speed, min_speed_, max_speed_);
    velocity_ = dir * speed;

    // std::cout << "Clamped Velocity: (" << velocity_.x << ", " << velocity_.y << ", " << velocity_.z << ")\n";
    // std::cout << "Actual Speed: " << speed << std::endl << std::endl;

    pos_ += velocity_ * delta_time_;

    glm::mat4 rot_mat_xz = glm::rotate(atan2f(velocity_.x, velocity_.z), glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 rot_mat_y = glm::rotate(-asinf(velocity_.y), glm::vec3(1.f, 0.f, 0.f));
    rot_mat_ = rot_mat_xz * rot_mat_y;

    acceleration_ = glm::vec3(0.f);
}