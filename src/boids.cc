#include <algorithm>

#include "boids.hh"

std::vector<Boid> Boid::get_neighbours(const std::vector<Boid>& boids) {
    // Gets the neihbours of a boid, based on its perception distance
    std::vector<Boid> neighbours;
    for (auto& boid : boids) {
        if (boid.pos_ != pos_ && glm::distance(boid.pos_, pos_) < perception_distance_)
            neighbours.emplace_back(boid);
    }

    return neighbours;
}

glm::vec3 Boid::align_force(const std::vector<Boid>& neighbours) {
    // Computes the alignment force of a boid (mean of the velocities of its neighbours)
    glm::vec3 avg_dir = velocity_;
    for (auto& neighbour: neighbours)
        avg_dir += neighbour.prev_velocity_;

    glm::vec3 align_force = -velocity_;
    avg_dir = avg_dir / (float) (neighbours.size() + 1);

    if (glm::length(avg_dir) > 0)
        align_force += glm::normalize(avg_dir);

    if (glm::length(align_force) > max_steer_)
        return glm::normalize(align_force) * max_steer_;

    return align_force;
}

glm::vec3 Boid::cohesion_force(const std::vector<Boid>& neighbours) {
    // Computes the cohesion force of a boid (towards center of mass of the boid and its neighbours)
    glm::vec3 center = pos_;
    for (auto& neighbour: neighbours)
        center += neighbour.prev_pos_;

    glm::vec3 cohesion_force(0.f);
    if (!neighbours.empty()) {
        center = center / (float) (neighbours.size() + 1);

        if (glm::length(center - pos_) > 0)
            cohesion_force += glm::normalize(center - pos_);

        if (glm::length(cohesion_force) > max_steer_)
            return glm::normalize(cohesion_force) * max_steer_;
    }

    return cohesion_force;
}

glm::vec3 Boid::separation_force(const std::vector<Boid>& neighbours) {
    // Computes the separation force of a boid (away from the mean positions of its neighbours, pondered by
    // the distance)
    glm::vec3 avg_dir(0.f);
    for (auto& neighbour: neighbours)
        avg_dir += ((pos_ - neighbour.prev_pos_) / glm::distance(pos_, neighbour.prev_pos_));

    glm::vec3 separation_force(0.f);
    if (!neighbours.empty()) {
        avg_dir = avg_dir / (float) neighbours.size();

        if (glm::length(avg_dir) > 0)
            separation_force += glm::normalize(avg_dir);

        if (glm::length(separation_force) > max_steer_)
            return glm::normalize(separation_force) * max_steer_;
    }

    return separation_force;
}

glm::vec3 Boid::avoid_objects(const std::vector<Object>& objects) {
    // Additional force to avoid the static objects of the scene
    glm::vec3 avg_dir(0.f);
    size_t count = 0;

    for (auto& obj: objects) {
        float distance = glm::distance(pos_, obj.pos_) - (obj.size_ / 2);
        if (distance > perception_distance_)
            continue;

        avg_dir += ((pos_ - obj.pos_) / glm::distance(pos_, obj.pos_));
        count++;
    }

    glm::vec3 avoid_force(0.f);
    if (count > 0) {
        avg_dir = avg_dir / (float) count;

        if (glm::length(avg_dir) > 0)
            avoid_force += glm::normalize(avg_dir);

        // std::cout << "Avoidance: (" << avoid_force.x << ", " << avoid_force.y << ", " << avoid_force.z << ")\n";

        if (glm::length(avoid_force) > max_steer_)
            return glm::normalize(avoid_force) * max_steer_;
    }

    return avoid_force;
}

glm::vec3 Boid::restrict_boundaries() {
    // Additional force to prevent the boids from going under the floor or above the sea level
    float y_dir = 0;
    if (pos_.y > (-50.f)) {
        y_dir = (pos_.y / glm::distance(pos_.y, -30.f));
    }

    if (pos_.y < (-150.f + 2.f) && pos_.z < 30.f) {
        y_dir = -(pos_.y / glm::distance(pos_.y, -150.f));
    }

    glm::vec3 avoid_force(0.f);
    if (y_dir != 0) {
        avoid_force = glm::vec3(0.f, y_dir, 0.f);

        if (velocity_.x < .001f && velocity_.z < .001f)
            avoid_force.z += y_dir;

        // std::cout << "Avoidance: (" << avoid_force.x << ", " << avoid_force.y << ", " << avoid_force.z << ")\n";

        if (glm::length(avoid_force) > max_steer_)
            return glm::normalize(avoid_force) * max_steer_;
    }

    return avoid_force;
}

void Boid::move(const std::vector<Boid>& boids, const std::vector<Object>& objects, float elapsed_time) {
    // Compute elapsed time between to call to move(), to compensate a potential low frame rate
    delta_time_ = (elapsed_time - last_time_) / 1000.f;
    last_time_ = elapsed_time;

    // Stores position and velocity before updating it, so that the other can still access the correct values
    prev_pos_ = pos_;
    prev_velocity_ = velocity_;

    // Apply forces
    auto neighbours = get_neighbours(boids);
    acceleration_ += align_force(neighbours) * align_weight_;
    acceleration_ += cohesion_force(neighbours) * cohesion_weight_;
    acceleration_ += separation_force(neighbours) * separation_weight_;
    acceleration_ += avoid_objects(objects) * avoid_weight_;
    acceleration_ += restrict_boundaries() * restrict_weight_;

    velocity_ += acceleration_ * delta_time_;

    // Clamp the speed (velocity norm), to prevent the boids from going too fast or too slow
    float speed = glm::length(velocity_);
    glm::vec3 dir = velocity_ / speed;
    speed = std::clamp(speed, min_speed_, max_speed_);
    velocity_ = dir * speed;

    pos_ += velocity_ * delta_time_;

    // Update rotation matrix of the boid, based on the direction of its updated velocity
    glm::mat4 rot_mat_xz = glm::rotate(atan2f(dir.x, dir.z), glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 rot_mat_y = glm::rotate(-asinf(std::clamp(dir.y, -1.f, 1.f)), glm::vec3(1.f, 0.f, 0.f));
    rot_mat_ = rot_mat_xz * rot_mat_y;

    acceleration_ = glm::vec3(0.f);
}