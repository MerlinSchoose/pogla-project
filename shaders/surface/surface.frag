#version 450

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
vec4 sky_color = vec4(0.5, 0.7, .9, 1);

layout(location=0) out vec4 output_color;

float fresnel(vec3 I, vec3 norm, float eta) {
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
    /*
    float caustic_intensity = clamp(exp(.01 * depth), 0, 1);
    float fog_intensity = clamp(exp(.0045 * cameraDepth), 0, 1);
    float sun_intensity = clamp(exp(.03 * cameraDepth), 0, 1);
    float blue_intensity = clamp(exp(.025 * cameraDepth), 0, 1);
    float darkness_intensity = clamp(exp(.0045 * cameraDepth), 0, 1);
    float Kr = fresnel(vec3(0, -1, 0), )

    //float sun = clamp(dot(normalize(ray), vec3(0, 1, 0)), 0, 1);

    float fogv = clamp((1000 * fog_intensity - length(ray)) / (1000 * fog_intensity - 10 * fog_intensity), 0, 1);
    fog_color = mix(fog_color, surface_color, blue_intensity);
    fog_color = mix(fog_color, vec4(vec3(0), 1), 1 - darkness_intensity);
    output_color = mix(fog_color, output_color, fogv * fogv * fogv);
    //output_color = mix(output_color, vec4(1), pow(sun, 6 / sun_intensity));
    output_color = vec4(output_color.xyz, z);
    */
    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    vec3 rayn = normalize(ray);
    vec3 refracted = normalize(refract(vec3(0, -1, 0), vNormal, 3/4f));
    /*
    vec3 refracted = normalize(refract(rayn, -vNormal, 3/4.f));
    float Kr = fresnel(rayn, -vNormal, 4/3.f);
    float light_intensity = clamp(dot(refracted, vec3(0, 1, 0)), 0, 1);
    output_color = clamp(surface_color * Kr + (1 - Kr) * mix(sky_color, vec4(1), light_intensity * light_intensity * light_intensity), 0, 1);
    */
    output_color = vec4(surface_color.rgb + pow(dot(refracted, -rayn), 2), z);
}