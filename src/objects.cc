#include "objects.hh"

Object::Object(const std::vector<Object> &objects,
               const std::vector<Vao *> &vaos,
               const glm::vec3 min_pos, const glm::vec3 max_pos,
               const float min_scale, const float max_scale,
               const float min_rot, const float max_rot,
               const glm::vec3 vec_rot, const float size) {
    vaos_ = vaos;
    vec_rot_ = vec_rot;

    srand((unsigned) time(nullptr));

    float x_pos_range = max_pos.x - min_pos.x;
    float y_pos_range = max_pos.y - min_pos.y;
    float z_pos_range = max_pos.z - min_pos.z;

    float rot_range = max_rot - min_rot;
    float scale_range = max_scale - min_scale;

    bool is_valid = false;
    glm::vec3 new_pos;

    for (size_t j = 0; !is_valid && j < 100; ++j) {
        float x_pos = ((float) rand() / (float) RAND_MAX) * x_pos_range + min_pos.x;
        float y_pos = ((float) rand() / (float) RAND_MAX) * y_pos_range + min_pos.y;
        float z_pos = ((float) rand() / (float) RAND_MAX) * z_pos_range + min_pos.z;

        scale_ = ((float) rand() / (float) RAND_MAX) * scale_range + min_scale;

        pos_ = glm::vec3(x_pos, y_pos, z_pos);

        is_valid = true;
        for (auto &obj: objects) {
            if (glm::distance(obj.pos_, pos_) < size * scale_)
                is_valid = false;
        }

        rot_ = ((float) rand() / (float) RAND_MAX) * rot_range + min_rot;
    }
}

void Object::draw(GLint mv_loc) {
    auto model_matrix = glm::rotate(glm::scale(glm::translate(pos_), glm::vec3(scale_)),
                               glm::radians(rot_), vec_rot_);
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: vaos_) {
        vao->draw();
    }
}
