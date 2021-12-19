#pragma once

#include <utility>

#include "vao.hh"

class Object {
public:
    Object(std::vector<Vao *> vaos,
           const glm::vec3 pos,
           const float scale,
           const float rot,
           const glm::vec3 vec_rot)
            : vaos_(std::move(vaos)),
              pos_(pos),
              scale_(scale),
              rot_(glm::radians(rot)),
              vec_rot_(vec_rot)
    { };

    Object(const std::vector<Object> &objects,
           const std::vector<Vao *> &vaos,
           glm::vec3 min_pos, glm::vec3 max_pos,
           float min_scale, float max_scale,
           float min_rot, float max_rot,
           glm::vec3 vec_rot, float size);

    void draw(GLint mv_loc);

protected:
    std::vector<Vao *> vaos_;

    glm::vec3 pos_;
    float scale_;
    float rot_;
    glm::vec3 vec_rot_;
};