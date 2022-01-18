#include "program.hh"
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>

#define LOG_MAX_SIZE 4096

program::program() {
    is_ready_ = false;
    log_ = nullptr;

    id = glCreateProgram();TEST_OPENGL_ERROR();
}

program::~program() {
    is_ready_ = false;
    delete[] log_;

    glDeleteProgram(id);TEST_OPENGL_ERROR();
}

program *program::make_program(const char *vertex_shader_src, const char *fragment_shader_src, const char *geometry_shader_src, const char *tesscontrol_shader, const char *tesseval_shader) {
    auto myprogram = new program;

    auto vertex_shader = std::shared_ptr<shader>(shader::make_shader(vertex_shader_src, GL_VERTEX_SHADER));

    if (!vertex_shader->is_ready())
    {
        myprogram->log_ = vertex_shader->get_log();
        return myprogram;
    }
    auto fragment_shader = std::shared_ptr<shader>(shader::make_shader(fragment_shader_src, GL_FRAGMENT_SHADER));

    if (!fragment_shader->is_ready())
    {
        myprogram->log_ = fragment_shader->get_log();
        return myprogram;
    }

    if (tesscontrol_shader) {
        auto tessc_shader = std::shared_ptr<shader>(shader::make_shader("../shaders/surface/caustic.tesscontrol", GL_TESS_CONTROL_SHADER));
        if (!tessc_shader->is_ready())
        {
            myprogram->log_ = tessc_shader->get_log();
            return myprogram;
        }
        myprogram->attach(*tessc_shader);
    }

    if (tesseval_shader) {
        auto tesse_shader = std::shared_ptr<shader>(shader::make_shader("../shaders/surface/caustic.tesseval", GL_TESS_EVALUATION_SHADER));
        if (!tesse_shader->is_ready())
        {
            myprogram->log_ = tesse_shader->get_log();
            return myprogram;
        }
        myprogram->attach(*tesse_shader);
    }

    if (geometry_shader_src) {
        auto geometry_shader = std::shared_ptr<shader>(shader::make_shader(geometry_shader_src, GL_GEOMETRY_SHADER));

        if (!geometry_shader->is_ready())
        {
            myprogram->log_ = geometry_shader->get_log();
            return myprogram;
        }
        myprogram->attach(*geometry_shader);
    }


    myprogram->attach(*vertex_shader);
    myprogram->attach(*fragment_shader);

    myprogram->link();

    return myprogram;
}

char *program::get_log() {
    return log_;
}

bool program::is_ready() {
    return is_ready_;
}

void program::use() {
    glUseProgram(id);TEST_OPENGL_ERROR();
}

void program::attach(const shader &shd) {
    glAttachShader(id, shd.id);TEST_OPENGL_ERROR();
}

void program::link() {
    glLinkProgram(id);TEST_OPENGL_ERROR();

    GLint program_linked;
    glGetProgramiv(id, GL_LINK_STATUS, &program_linked);TEST_OPENGL_ERROR();
    if (program_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        auto *message = new GLchar[LOG_MAX_SIZE];
        glGetProgramInfoLog(id, LOG_MAX_SIZE, &log_length, message);TEST_OPENGL_ERROR();

        log_ = message;
    }
    else
        is_ready_ = true;
}
