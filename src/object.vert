#version 450

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 cameraPos;

in vec3 position;
in vec2 uv;

out float depth;
out float cameraDepth;
out vec3 ray;

out vec2 vUv;

void main() {
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);

    ray = position - cameraPos;
    depth = position.y;
    cameraDepth = cameraPos.y;
	//texture
    vUv = uv;
}