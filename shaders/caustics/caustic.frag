#version 450

in vec3 intensity;

layout(location=0) out vec4 output_color;

void main() {

    // On stocke la depth pour faire un test manuel plus tard.
    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    output_color = vec4(intensity.xxx, z);
}