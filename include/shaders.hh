#pragma once

#include "opengl.hh"

class shader {
public:
    explicit shader(GLuint type);
    ~shader() {
        glDeleteShader(id);TEST_OPENGL_ERROR();
    }
    static shader *make_shader(const char *shader_file, GLuint type);

    int compile();
    int source(const char *shader_file);
    char *get_log() const;

    bool is_ready() const {
        return is_ready_;
    }

    GLuint id;
    GLuint type;
private:
    bool is_ready_ = false;
};

