#version 450

uniform sampler2D texture_sampler;

uniform vec2 uv_multiplier = vec2(1);

in vec2 vUv;

layout(location=0) out vec4 output_color;

void main() {
    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    output_color = clamp(texture(texture_sampler, vUv * uv_multiplier), 0, 1);
    output_color = vec4(output_color.xyz, z);

}