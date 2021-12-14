#include <fstream>
#include <sstream>
#include <iostream>

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

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);TEST_OPENGL_ERROR();
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);TEST_OPENGL_ERROR();

    std::string vertex_shader_str;
    std::ifstream vertex_shader_stream(vertex_shader_src, std::ios::in);
    if(vertex_shader_stream.is_open()){
        std::stringstream str_buffer;
        str_buffer << vertex_shader_stream.rdbuf();
        vertex_shader_str = str_buffer.str();
        vertex_shader_stream.close();
    }
    else {
        std::cout << "Cannot access " << vertex_shader_src << ": No such file or directory\n";
        return myprogram;
    }

    std::string fragment_shader_str;
    std::ifstream fragment_shader_stream(fragment_shaders, std::ios::in);
    if(fragment_shader_stream.is_open()){
        std::stringstream str_buffer;
        str_buffer << fragment_shader_stream.rdbuf();
        fragment_shader_str = str_buffer.str();
        fragment_shader_stream.close();
    }
    else {
        std::cout << "Cannot access " << fragment_shaders << ": No such file or directory\n";
        return myprogram;
    }

    const char *vertex_shader_chars = vertex_shader_str.c_str();
    const char *fragment_shader_chars = fragment_shader_str.c_str();

    glShaderSource(vertex_shader, 1, &vertex_shader_chars, nullptr);TEST_OPENGL_ERROR();
    glShaderSource(fragment_shader, 1, &fragment_shader_chars, nullptr);TEST_OPENGL_ERROR();

    glCompileShader(vertex_shader);TEST_OPENGL_ERROR();

    GLint vertex_compiled;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_compiled);TEST_OPENGL_ERROR();
    if (vertex_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        auto *message = new GLchar[LOG_MAX_SIZE];
        glGetShaderInfoLog(vertex_shader, LOG_MAX_SIZE, &log_length, message);TEST_OPENGL_ERROR();

        myprogram->log_ = message;
        return myprogram;
    }

    glCompileShader(fragment_shader);TEST_OPENGL_ERROR();

    GLint fragment_compiled;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &vertex_compiled);TEST_OPENGL_ERROR();
    if (vertex_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        auto *message = new GLchar[LOG_MAX_SIZE];
        glGetShaderInfoLog(fragment_shader, LOG_MAX_SIZE, &log_length, message);TEST_OPENGL_ERROR();

        myprogram->log_ = message;
        return myprogram;
    }

    glAttachShader(myprogram->id, vertex_shader);TEST_OPENGL_ERROR();
    glAttachShader(myprogram->id, fragment_shader);TEST_OPENGL_ERROR();

    glLinkProgram(myprogram->id);TEST_OPENGL_ERROR();

    GLint program_linked;
    glGetProgramiv(myprogram->id, GL_LINK_STATUS, &program_linked);TEST_OPENGL_ERROR();
    if (program_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        auto *message = new GLchar[LOG_MAX_SIZE];
        glGetProgramInfoLog(myprogram->id, LOG_MAX_SIZE, &log_length, message);TEST_OPENGL_ERROR();

        myprogram->log_ = message;
        return myprogram;
    }

    glDeleteShader(vertex_shader);TEST_OPENGL_ERROR();
    glDeleteShader(fragment_shader);TEST_OPENGL_ERROR();

    myprogram->is_ready_ = true;

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