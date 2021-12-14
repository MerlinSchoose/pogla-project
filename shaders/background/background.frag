#version 450

in float depth;
in float cameraDepth;
in vec3 ray;

const float FOG_MIN = .6;
const float FOG_MAX = 1.;
const float black_depth = -1000;
const float caustic_depth = -500;
vec4 fog_color = vec4(.0, .25, .8, 1);
vec4 surface_color = vec4(.0, .55, .7, 1);

vec3 light_color = vec3(1.0, 1.0, 1.0);
vec3 light_position = vec3(0.0, 0.0, 150.);

layout(location=0) out vec4 output_color;

float fresnel(vec3 I, vec3 norm, float eta)
{
    float cos_init = dot(normalize(norm), normalize(I));
    float sin_out;
    if (cos_init > 0)
    sin_out = sqrt(max(0., 1 - cos_init * cos_init)) * eta;
    else {
        sin_out = sqrt(max(0., 1 - cos_init * cos_init)) / eta;
        cos_init = -cos_init;
    }

    if (sin_out >= 1) return 1;

    float cos_out = sqrt(max(0., 1 - sin_out * sin_out));
    float Rs = (eta * cos_init - cos_out) / (eta * cos_init + cos_out);
    float Rp = (cos_init - eta * cos_out) / (cos_init + eta * cos_out);
    return (Rs * Rs + Rp * Rp) / 2;
}

float fog(float depth) {
    return clamp(1 - (FOG_MAX - depth) / (FOG_MAX - FOG_MIN), 0, 1);
}

void main() {

    float sun_intensity = clamp(exp(.025 * cameraDepth), 0, 1);
    float fogv2 = clamp(exp(.0045 * cameraDepth), 0, 1);
    fog_color = mix(fog_color, surface_color, sun_intensity);
    output_color = mix(fog_color, vec4(vec3(0), 1), 1 - fogv2);
}
