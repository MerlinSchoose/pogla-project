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

    void draw();

    static Vao *make_vao(GLuint vertex_location,
                         std::vector<GLfloat> vertices,
                         GLuint texture_id = -1,
                         GLuint uv_location = -1,
                         std::vector<GLfloat> uv = std::vector<GLfloat>(),
                         std::vector<GLuint> indices = std::vector<GLuint>(),
                         GLuint normal_location = -1,
                         std::vector<GLfloat> normals = std::vector<GLfloat>());

    GLuint id;
    std::vector<GLuint> vbo_ids;
    GLuint ebo_id = -1;
    size_t draw_size = 0;
    GLuint texture_id = -1;
    bool using_indices = false;
};

#endif //MAIN_VAO_HH