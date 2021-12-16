#pragma once

#include "objects.hh"

class Boid : public Object {
public:
    Boid(const std::vector<Object> &objects,
         const std::vector<Vao *> &vaos,
         glm::vec3 min_pos, glm::vec3 max_pos,
         float min_scale, float max_scale,
         float min_rot, float max_rot,
         glm::vec3 vec_rot, float size)
            : Object(objects, vaos,
                     min_pos, max_pos,
                     min_scale, max_scale,
                     min_rot, max_rot,
                     vec_rot, size) {
        vec_dir_ = glm::normalize(glm::vec3(1 - sinf(rot_), 0.f, cosf(rot_)));
        std::cout << rot_ << std::endl;
        std::cout << "(" << vec_dir_.x << ", " << vec_dir_.y << ", " << vec_dir_.z << ")" << std::endl;
    }

    void move(float speed);

private:
    glm::vec3 vec_dir_;
};