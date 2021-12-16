#include "boids.hh"

void Boid::move(float speed) {
    // float rot_offset = ((float) rand() / (float) RAND_MAX) * 2.f - 1.f;
    // rot_ += rot_offset;

    // vec_dir_ = glm::rotate(vec_dir_, rot_, vec_rot_);

    pos_ += speed * vec_dir_;
}