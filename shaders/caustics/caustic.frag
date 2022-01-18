#version 450

uniform mat4 model_matrix;
uniform sampler2D depth_texture;

uniform vec3 cameraPos;

in vec3 intensity;
in vec3 pos;

const float FOG_MIN = .3;
const float FOG_MAX = 1.;
const float black_depth = -1000;
const float caustic_depth = -500;
vec4 fog_color = vec4(.0, .25, .8, 1);
vec4 surface_color = vec4(.0, .55, .7, 1);

vec3 light_color = vec3(1.0, 1.0, 1.0);
vec3 light_position = vec3(0.0, 0.0, 150.);

layout(location=0) out vec4 output_color;

void main() {
    /*
    vec3 ray = (model_matrix * vec4(pos, 1)).xyz - cameraPos;
    float depth = (model_matrix * vec4(pos, 1)).y;
    float cameraDepth = cameraPos.y;

    float caustic_intensity = clamp(exp(.01 * depth), 0, 1);
    float fog_intensity = clamp(exp(.0045 * cameraDepth), 0, 1);
    float sun_intensity = clamp(exp(.03 * cameraDepth), 0, 1);
    float blue_intensity = clamp(exp(.025 * cameraDepth), 0, 1);
    float darkness_intensity = clamp(exp(.0045 * cameraDepth), 0, 1);

    float fogv = clamp((1000 * fog_intensity - length(ray)) / (1000 * fog_intensity - 10 * fog_intensity), 0, 1);
    //float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    fog_color = mix(fog_color, surface_color, blue_intensity);
    fog_color = mix(fog_color, vec4(vec3(0), 1), 1 - darkness_intensity);
    output_color = mix(fog_color, output_color, fogv * fogv * fogv);
    //output_color = mix(output_color, vec4(1), pow(sun, 6 / sun_intensity));
    output_color = vec4(output_color.xyz, z);
    */
    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    output_color = vec4(intensity.xxx, z);
}