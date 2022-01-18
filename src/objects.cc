#include "objects.hh"

Object::Object(const std::vector<Object> &objects,
               const std::vector<Vao *> &vaos,
               const glm::vec3 min_pos, const glm::vec3 max_pos,
               const float min_rot_x, const float max_rot_x,
               const float min_rot_y, const float max_rot_y,
               const float min_rot_z, const float max_rot_z,
               const float min_scale, const float max_scale, const float size) {
    // Object random initialisation (from ranges of values)
    vaos_ = vaos;

    const float x_pos_range = max_pos.x - min_pos.x;
    const float y_pos_range = max_pos.y - min_pos.y;
    const float z_pos_range = max_pos.z - min_pos.z;

    const float scale_range = max_scale - min_scale;

    bool is_valid = false;
    glm::vec3 new_pos;

    // Loop to try new positions, until a valid one is found (where there are no other objects)
    for (size_t j = 0; !is_valid && j < 100; ++j) {
        // Random position and scale generation
        const float x_pos = ((float) rand() / (float) RAND_MAX) * x_pos_range + min_pos.x;
        const float y_pos = ((float) rand() / (float) RAND_MAX) * y_pos_range + min_pos.y;
        const float z_pos = ((float) rand() / (float) RAND_MAX) * z_pos_range + min_pos.z;

        scale_ = ((float) rand() / (float) RAND_MAX) * scale_range + min_scale;
        size_ = size * scale_;

        pos_ = glm::vec3(x_pos, y_pos, z_pos);

        // Test if an already existing object is too close
        is_valid = true;
        for (auto &obj: objects) {
            if (glm::distance(obj.pos_, pos_) < (size_ + obj.size_) / 2)
                is_valid = false;
        }
    }

    // If no positions where found in 100 tries, we consider that there is no room left to spawn a new object
    if (!is_valid)
        throw std::invalid_argument("An object could not spawn due to lack of space.");

    // Random rotation matrix generation
    const float x_rot_range = max_rot_x - min_rot_x;
    const float y_rot_range = max_rot_y - min_rot_y;
    const float z_rot_range = max_rot_z - min_rot_z;

    const float rot_x = ((float) rand() / (float) RAND_MAX) * x_rot_range + min_rot_x;
    const float rot_y = ((float) rand() / (float) RAND_MAX) * y_rot_range + min_rot_y;
    const float rot_z = ((float) rand() / (float) RAND_MAX) * z_rot_range + min_rot_z;

    rot_mat_ = (
            glm::rotate(glm::mat4(1.f), glm::radians(rot_x), glm::vec3(1.f, 0.f, 0.f))
            * glm::rotate(glm::mat4(1.f), glm::radians(rot_y), glm::vec3(0.f, 1.f, 0.f))
            * glm::rotate(glm::mat4(1.f), glm::radians(rot_z), glm::vec3(0.f, 0.f, 1.f))
    );
}

void Object::draw(GLint mv_loc, GLint vert_loc, GLint uv_loc, GLint normal_loc, GLuint tex) {
    // Draw an object: builds a model matrix to translate, rotate and rescale the object
    auto model_matrix = (
            glm::translate(pos_)
            * rot_mat_
            * glm::scale(glm::mat4(1.f), glm::vec3(scale_))
    );

    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();
    for (auto vao: vaos_) {
        vao->draw(vert_loc, uv_loc, normal_loc, tex);
    }
}