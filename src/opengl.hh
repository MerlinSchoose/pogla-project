//
// Created by simon on 7/14/21.
//

#ifndef MAIN_OPENGL_HH
#define MAIN_OPENGL_HH

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include <glm/ext.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

#define TEST_OPENGL_ERROR()                                                             \
  do {									\
    GLenum err = glGetError();					                        \
    if (err != GL_NO_ERROR) std::cerr << "OpenGL ERROR! " << __LINE__ << " " << gluErrorString(err) << std::endl;      \
  } while(0)

#endif //MAIN_OPENGL_HH
