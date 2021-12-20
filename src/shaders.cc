#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>

#include "string"
#include "shaders.hh"

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

program *program::make_program(const char *vertex_shader_src, const char *fragment_shaders) {
    auto myprogram = new program;

    auto vertex_shader = std::shared_ptr<shader>(shader::make_shader(vertex_shader_src, GL_VERTEX_SHADER));

    if (!vertex_shader->is_ready())
    {
        myprogram->log_ = vertex_shader->get_log();
        return myprogram;
    }
    auto fragment_shader = std::shared_ptr<shader>(shader::make_shader(fragment_shaders, GL_FRAGMENT_SHADER));

    if (!fragment_shader->is_ready())
    {
        myprogram->log_ = fragment_shader->get_log();
        return myprogram;
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

shader::shader(GLuint type) {
    id = glCreateShader(type);TEST_OPENGL_ERROR();
}

int shader::compile() {

    glCompileShader(id);TEST_OPENGL_ERROR();

    GLint compiled;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);TEST_OPENGL_ERROR();
    is_ready_ = compiled;
    return compiled;
}

shader *shader::make_shader(const char *shader_file, GLuint type) {
    auto res = new shader(type);
    if (res->source(shader_file) != GL_TRUE) {
        std::cout << "Cannot access " << shader_file << ": No such file or directory\n";
        return res;
    }
    res->compile();
    return res;
}

int shader::source(const char *shader_file) {
    std::string shader_str;
    std::ifstream shader_stream(shader_file, std::ios::in);
    if(shader_stream.is_open()){
        std::stringstream str_buffer;
        str_buffer << shader_stream.rdbuf();
        shader_str = str_buffer.str();
        shader_stream.close();
    }
    else {
        return GL_FALSE;
    }

    const char *shader_chars = shader_str.c_str();
    glShaderSource(id, 1, &shader_chars, nullptr);TEST_OPENGL_ERROR();
    return GL_TRUE;
}

char *shader::get_log() const {
    GLsizei log_length = 0;
    auto *message = new GLchar[LOG_MAX_SIZE];
    glGetShaderInfoLog(id, LOG_MAX_SIZE, &log_length, message);TEST_OPENGL_ERROR();
    return message;
}
