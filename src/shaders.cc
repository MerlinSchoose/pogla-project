#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>

#include "shaders.hh"

#define LOG_MAX_SIZE 4096



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
