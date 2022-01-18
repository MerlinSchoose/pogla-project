#version 450

uniform mat4 model_matrix;
uniform vec3 cameraPos;

in vec3 intensity;
in vec3 pos;
in vec3 ray;

const float FOG_MIN = .3;
const float FOG_MAX = 1.;
const float black_depth = -1000;
const float caustic_depth = -500;
vec4 fog_color = vec4(.0, .25, .8, 1);
vec4 surface_color = vec4(.0, .55, .7, 1);

vec3 light_color = vec3(1.0, 1.0, 1.0);
vec3 light_position = vec3(0.0, 0.0, 150.);

float attenuation = 0.1;
float amplification = 1.4;
float caustic_intensity(vec3 pos) {
    return amplification * exp(-attenuation * -pos.y / 90.f);
}

float mie_phase(float angle) {
    float g = -0.95;
    float g2 = g * g;
    return (3 * (1 - g2)) / (2 * (2 + g2)) * (1 + angle * angle) / pow(1 + g2 - 2 * g * angle, 3.f/2.f);
}

float godray_intensity(vec3 view, vec3 ray, vec3 pos) {
    float mie = mie_phase((-dot(normalize(view), normalize(ray))));
    return caustic_intensity(pos) * mie * exp(-attenuation * length(view) / 50.f);
}

layout(location=0) out vec4 output_color;

void main() {
    vec3 view = pos - cameraPos;
    float depth = pos.y;
    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    float res = godray_intensity(view, ray, pos);
    output_color = vec4(res.xxx, z);
}
