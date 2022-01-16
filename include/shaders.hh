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

class program {
public:
    program();
    ~program();

    static program *make_program(const char *vertex_shader_src, const char *fragment_shaders, const char *geometry_shader = nullptr, const char *tesscontrol_shader = nullptr, const char *tesseval_shader = nullptr);
    char *get_log();
    bool is_ready();
    void use();
    void attach(const shader &shd);
    void link();

    GLuint id;
private:
    bool is_ready_;
    char *log_;
};
