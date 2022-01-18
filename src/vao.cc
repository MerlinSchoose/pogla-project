#include "vao.hh"

Vao *Vao::make_vao(GLint vertex_location,
                   const std::vector<GLfloat>& vertices,
                   GLuint texture_id,
                   GLint uv_location,
                   const std::vector<GLfloat>& uv,
                   const std::vector<GLuint>& indices,
                   GLint normal_location,
                   const std::vector<GLfloat>& normals) {
    auto vao = new Vao;
    glBindVertexArray(vao->id);TEST_OPENGL_ERROR();
    if (!vertices.empty()) vao->vbo_ids.push_back(-1);
    if (!uv.empty()) vao->vbo_ids.push_back(-1);
    if (!normals.empty()) vao->vbo_ids.push_back(-1);
    glGenBuffers(vao->vbo_ids.size(), vao->vbo_ids.data());TEST_OPENGL_ERROR();

    vao->draw_size = vertices.size();
    if (!vertices.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, vao->vbo_ids[0]);TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);TEST_OPENGL_ERROR();
    }
    if (vertex_location != -1) {
        glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(vertex_location);TEST_OPENGL_ERROR();
    }
    vao->vert_loc = vertex_location;

    if (!uv.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, vao->vbo_ids[1]);TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(float), uv.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
    }
    if (uv_location != -1) {
        glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(uv_location);TEST_OPENGL_ERROR();
    }
    vao->uv_loc = uv_location;

    if (!normals.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, vao->vbo_ids[2]);TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_DYNAMIC_DRAW);TEST_OPENGL_ERROR();
    }
    if (normal_location != -1) {
        glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(normal_location);TEST_OPENGL_ERROR();
    }
    vao->norm_loc = normal_location;

    if (!indices.empty()) {
        vao->draw_size = indices.size();
        vao->using_indices = true;
        glGenBuffers(1, &vao->ebo_id);TEST_OPENGL_ERROR();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->ebo_id);TEST_OPENGL_ERROR();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
    }

    glBindVertexArray(0);TEST_OPENGL_ERROR();
    if (texture_id != -1) {
        vao->texture_id = texture_id;
    }

    return vao;
}

void Vao::draw(GLint vertex_loc, GLint uv_loc, GLint normal_loc, GLint tex) {
    glBindVertexArray(id);TEST_OPENGL_ERROR();

    if (vertex_loc != -1) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[0]);TEST_OPENGL_ERROR();
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(vertex_loc);TEST_OPENGL_ERROR();
    }
    else if (vert_loc != -1) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[0]);TEST_OPENGL_ERROR();
        glVertexAttribPointer(vert_loc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(vert_loc);TEST_OPENGL_ERROR();
    }

    if (uv_loc != -1) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[1]);TEST_OPENGL_ERROR();
        glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(uv_loc);TEST_OPENGL_ERROR();
    } else if (this->uv_loc != -1) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[1]);TEST_OPENGL_ERROR();
        glVertexAttribPointer(this->uv_loc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(this->uv_loc);TEST_OPENGL_ERROR();
    }

    if (normal_loc != -1) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[2]);TEST_OPENGL_ERROR();
        glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(normal_loc);TEST_OPENGL_ERROR();
    } else if (this->norm_loc != -1) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[2]);TEST_OPENGL_ERROR();
        glVertexAttribPointer(this->norm_loc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(this->norm_loc);TEST_OPENGL_ERROR();
    }

    if (tex != -1 && tex != -2) {
        glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
        glBindTexture(GL_TEXTURE_2D, tex);TEST_OPENGL_ERROR();
    } else if (tex == -2) {
    } else if (texture_id != -1) {
        glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
        glBindTexture(GL_TEXTURE_2D, texture_id);TEST_OPENGL_ERROR();
    }

    if (using_indices) {
        glDrawElements(GL_TRIANGLES, draw_size, GL_UNSIGNED_INT, 0);TEST_OPENGL_ERROR();
    }

    else {
        glDrawArrays(GL_TRIANGLES, 0, draw_size);TEST_OPENGL_ERROR();
    }

    glBindVertexArray(0);TEST_OPENGL_ERROR();
}
