#ifndef MAIN_VAO_HH
#define MAIN_VAO_HH

#include <vector>

#include "opengl.hh"

class Vao {
public:
    Vao() {
        id = 0;
        glGenVertexArrays(1, &id);
    }

    ~Vao() {
        glDeleteVertexArrays(1, &id);
    }

    void draw(GLint vertex_loc = -1, GLint uv_loc = -1, GLint normal_loc = -1, GLint texture_id = -1);

    static Vao *make_vao(GLint vertex_location,
                         const std::vector<GLfloat>& vertices,
                         GLuint texture_id = -1,
                         GLint uv_location = -1,
                         const std::vector<GLfloat>& uv = std::vector<GLfloat>(),
                         const std::vector<GLuint>& indices = std::vector<GLuint>(),
                         GLint normal_location = -1,
                         const std::vector<GLfloat>& normals = std::vector<GLfloat>());

    GLuint id;
    std::vector<GLuint> vbo_ids;
    GLuint ebo_id = -1;
    size_t draw_size = 0;
    GLuint texture_id = -1;
    GLint vert_loc = -1;
    GLint uv_loc = -1;
    GLint norm_loc = -1;
    bool using_indices = false;
};

#endif //MAIN_VAO_HH