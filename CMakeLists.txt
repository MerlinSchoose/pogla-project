cmake_minimum_required(VERSION 3.17)
project(main)

set(CMAKE_CXX_STANDARD 17)

find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(include ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

add_executable(main
        src/main.cc
        src/image.cc
        src/image_io.cc
        src/shaders.cc
        src/vao.cc
        src/camera.cc)

target_link_libraries( ${PROJECT_NAME} -lGLEW -lGL -lglut -lGLU)
