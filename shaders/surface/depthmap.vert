#version 450

uniform mat4 model_matrix;
uniform mat4 view_matrix_light;
uniform mat4 projection_matrix_light;

in vec3 position;

out float depth;
out vec4 vPos;

void main() {
    gl_Position = projection_matrix_light * view_matrix_light * model_matrix * vec4(position, 1.0);

    depth = gl_Position.z / gl_Position.w;
    vPos = model_matrix * vec4(position, 1.0);
}
