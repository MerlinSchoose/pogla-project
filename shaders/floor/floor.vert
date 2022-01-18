#version 450

in vec3 position;
in vec3 normal;
in vec2 uv;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 cameraPos;


out float depth;
out float cameraDepth;
out vec3 ray;

out vec2 vUv;
out vec3 vNormal;

void main() {
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);

    ray = (model_matrix * vec4(position, 1)).xyz - cameraPos;
    depth = (model_matrix * vec4(position, 1)).y;
    cameraDepth = cameraPos.y;

    //texture
    vUv = uv;

    vNormal = normal;
}