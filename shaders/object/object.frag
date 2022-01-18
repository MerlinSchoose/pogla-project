#version 450

uniform sampler2D texture_sampler;

uniform vec2 uv_multiplier = vec2(1);

in vec2 vUv;
in float depth;
uniform vec3 cameraPos;
in vec3 ray;

const float FOG_MIN = .3;
const float FOG_MAX = 1.;
const float black_depth = -1000;
const float caustic_depth = -500;
vec4 fog_color = vec4(.0, .25, .8, 1);
vec4 surface_color = vec4(.0, .55, .7, 1);

layout(location=0) out vec4 output_color;

float fog(float depth) {
    return clamp(1 - (FOG_MAX - depth) / (FOG_MAX - FOG_MIN), 0, 1);
}

void main() {
    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    output_color = clamp(texture(texture_sampler, vUv * uv_multiplier), 0, 1);
    output_color = vec4(output_color.xyz, z);

}