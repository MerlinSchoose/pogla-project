//
// Created by simon on 1/17/22.
//

#ifndef MAIN_PROGRAM_HH
#define MAIN_PROGRAM_HH

#include "shaders.hh"

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

#endif //MAIN_PROGRAM_HH
