#version 450

uniform sampler2D caustic_sampler;
uniform sampler2D floor_sampler;

in vec2 vUv;
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

vec3 refract(vec3 I, vec3 norm, float eta) {
    float cos_init = dot(normalize(I), normalize(norm));
    if (cos_init < 0) {
        eta = 1 / eta;
        cos_init = -cos_init;
    }
    else {
        norm = -norm;
    }

    float k = 1 - eta * eta * (1 - cos_init * cos_init);

    return eta * I + (eta * cos_init - sqrt(k)) * norm;
}

float fog(float depth) {
    return clamp(1 - (FOG_MAX - depth) / (FOG_MAX - FOG_MIN), 0, 1);
}

void main() {
    float caustic_intensity = clamp(exp(.005 * depth), 0, 1);
    float fog_intensity = clamp(exp(.0045 * cameraDepth), 0, 1);
    float sun_intensity = clamp(exp(.03 * cameraDepth), 0, 1);

    output_color = clamp(texture(floor_sampler, vUv) + vec4(vec3(texture(caustic_sampler, vUv).r), 1) * caustic_intensity * 0.6, 0, 1);
    float sun = clamp(dot(normalize(ray), vec3(0, 1, 0)), 0, 1);

    float fogv = clamp((1000 * fog_intensity - length(ray)) / (1000 * fog_intensity - 10 * fog_intensity), 0, 1);
    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    float darkness_intensity = clamp(exp(.0045 * cameraDepth), 0, 1);
    fog_color = mix(fog_color, surface_color, sun_intensity);
    fog_color = mix(fog_color, vec4(vec3(0), 1), 1 - darkness_intensity);
    output_color = mix(fog_color, output_color, fogv * fogv * fogv);
    output_color = vec4(output_color.rgb, 1.0);
}