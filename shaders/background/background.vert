#version 450

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 cameraPos;

in vec3 position;

void main() {
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);
}
