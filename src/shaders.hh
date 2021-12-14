#pragma once

#include "opengl.hh"

class program {
public:
    program();
    ~program();

    static program *make_program(const char *vertex_shader_src, const char *fragment_shaders);
    char *get_log();
    bool is_ready();
    void use();

    GLuint id;
private:
    bool is_ready_;
    char *log_;
};