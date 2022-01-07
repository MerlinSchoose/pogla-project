#version 450

in vec2 vUv;
in vec3 vNormal;
in vec4 vPos;
in float depth;
in float cameraDepth;
in vec3 ray;

layout(location=0) out vec4 output_color;

void main() {

    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    output_color = vec4(vPos.xyz, depth);
}