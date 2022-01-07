#version 450

uniform sampler2D depth_texture;
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
in float depth;
in vec3 newPos;
in vec3 oldPos;
in vec2 vUv;

layout(location=0) out vec4 output_color;

void main() {
    vec4 viewPos = (projection_matrix * view_matrix * vec4(oldPos, 1.0));
    vec2 coord = (viewPos.xy / viewPos.w * 0.5 + 0.5) / (vec2(2048) / vec2(1920, 1080));
    float oldArea = length(dFdx(oldPos)) * length(dFdy(oldPos));
    float newArea = length(dFdx(newPos)) * length(dFdy(newPos));

    float ratio;

    if (newArea == 0.) {
        ratio = 2.0e+20;
    } else {
        ratio = oldArea / newArea;
    }

    float causticsIntensity = ratio * 0.25;


    output_color = vec4(causticsIntensity);
}