#version 450

uniform sampler2D caustic_sampler;
uniform sampler2D floor_sampler;

in vec2 vUv;
in vec3 vNormal;
in float depth;
in float cameraDepth;
in vec3 ray;

const float FOG_MIN = .3;
const float FOG_MAX = 1.;
const float black_depth = -1000;
const float caustic_depth = -500;
vec4 fog_color = vec4(.0, .25, .8, 1);
vec4 surface_color = vec4(.0, .55, .7, 1);

vec3 light_color = vec3(1.0, 1.0, 1.0);
vec3 light_position = vec3(0.0, 0.0, 150.);

layout(location=0) out vec4 output_color;

float fog(float depth) {
    return clamp(1 - (FOG_MAX - depth) / (FOG_MAX - FOG_MIN), 0, 1);
}

void main() {
    float caustic_intensity = clamp(exp(.01 * depth), 0, 1);
    float fog_intensity = clamp(exp(.0045 * cameraDepth), 0, 1);
    float sun_intensity = clamp(exp(.03 * cameraDepth), 0, 1);
    float blue_intensity = clamp(exp(.025 * cameraDepth), 0, 1);
    float darkness_intensity = clamp(exp(.0045 * cameraDepth), 0, 1);

    output_color = clamp(texture(floor_sampler, vUv * 320) + vec4(vec3(texture(caustic_sampler, vUv * 32).r), 1) * caustic_intensity * 0.6, 0, 1);
    /*
    float sun = clamp(dot(normalize(ray), vec3(0, 1, 0)), 0, 1);

    float fogv = clamp((1000 * fog_intensity - length(ray)) / (1000 * fog_intensity - 10 * fog_intensity), 0, 1);
    fog_color = mix(fog_color, surface_color, blue_intensity);
    fog_color = mix(fog_color, vec4(vec3(0), 1), 1 - darkness_intensity);
    output_color = mix(fog_color, output_color, fogv * fogv * fogv);
    output_color = mix(output_color, vec4(1), pow(sun, 6 / sun_intensity));
    */
    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    output_color = vec4(output_color.xyz, z);
}