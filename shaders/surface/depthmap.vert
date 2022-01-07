#version 450

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 cameraPos;

in vec3 position;
in vec3 normal;
in vec2 uv;

out float depth;
out float cameraDepth;
out vec3 vNormal;
out vec4 vPos;
out vec3 ray;

out vec2 vUv;

void main() {
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);

    ray = (model_matrix * vec4(position, 1)).xyz - cameraPos;
    depth = gl_Position.z;
    cameraDepth = cameraPos.y;

    vPos = model_matrix * vec4(position, 1.0);

    //texture
    vUv = uv;

    //normal
    vNormal = normal;
}
