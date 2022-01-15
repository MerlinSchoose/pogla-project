#version 450

uniform sampler2D depth_texture;
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
in vec3 intensity;

layout(location=0) out vec4 output_color;

void main() {

    output_color = vec4(vec3(intensity), 0.1f);
    /*
    float oldArea = length(dFdx(oldPos)) * length(dFdy(oldPos));
    float newArea = length(dFdx(newPos)) * length(dFdy(newPos));

    float ratio;

    if (newArea == 0.) {
        ratio = 2.0e+20;
    } else {
        ratio = oldArea / newArea;
    }

    float causticsIntensity = ratio * 0.25;


    output_color = vec4(vec3(intensity), 1.0);
    */
}