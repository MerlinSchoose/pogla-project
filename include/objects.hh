#pragma once

#include <utility>

#include "vao.hh"

class Object {
public:
    Object(std::vector<Vao *> vaos,
           const glm::vec3 pos,
           const float rot_x,
           const float rot_y,
           const float rot_z,
           const float scale,
           const float size)
            : vaos_(std::move(vaos)),
              pos_(pos),
              scale_(scale),
              size_(size)
    {
        rot_mat_ = (
                glm::rotate(glm::mat4(1.f), glm::radians(rot_x), glm::vec3(1.f, 0.f, 0.f))
                * glm::rotate(glm::mat4(1.f), glm::radians(rot_y), glm::vec3(0.f, 1.f, 0.f))
                * glm::rotate(glm::mat4(1.f), glm::radians(rot_z), glm::vec3(0.f, 0.f, 1.f))
        );
    };

    Object(const std::vector<Object> &objects,
           const std::vector<Vao *> &vaos,
           glm::vec3 min_pos, glm::vec3 max_pos,
           float min_rot_x, float max_rot_x,
           float min_rot_y, float max_rot_y,
           float min_rot_z, float max_rot_z,
           float min_scale, float max_scale, float size);

    void draw(GLint mv_loc, GLint vert_loc = -1, GLint uv_loc = -1, GLint normal_loc = -1);

protected:
    std::vector<Vao *> vaos_;

    glm::vec3 pos_;
    glm::mat4 rot_mat_;
    float scale_;
    float size_;

    friend class Boid;
};