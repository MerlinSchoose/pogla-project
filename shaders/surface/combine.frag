#version 450

uniform sampler2DRect color_texture;
uniform sampler2DRect caustic_texture;
uniform sampler2DRect godray_texture;

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

vec3 light_color = vec3(1.0, 1.0, 1.0);
vec3 light_position = vec3(0.0, 0.0, 150.);


layout(location=0) out vec4 output_color;

void main() {

    float caustic_intensity = clamp(exp(.01 * depth), 0, 1);
    float fog_intensity = clamp(exp(.0045 * cameraDepth), 0, 1);
    float sun_intensity = clamp(exp(.03 * cameraDepth), 0, 1);
    float blue_intensity = clamp(exp(.025 * cameraDepth), 0, 1);
    float darkness_intensity = clamp(exp(.0045 * cameraDepth), 0, 1);

    vec4 color = texture(color_texture, ivec2(gl_FragCoord.xy));
    vec4 caustic = texture(caustic_texture, ivec2(gl_FragCoord.xy));
    vec4 godray = texture(godray_texture, ivec2(gl_FragCoord.xy));

    if (caustic.x != 0 && (caustic.w <= color.w) && caustic.x >= 0.0f) {
        vec3 white = vec3(1);
        vec3 c = sky_color.rgb * (caustic.r) + color.rgb;
        output_color = vec4(c, 1.0);
    } else {
        output_color = vec4(color.rgb, 1.0);
    }

    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    float fogv = clamp((1000 * fog_intensity - length(ray)) / (1000 * fog_intensity - 10 * fog_intensity), 0, 1);
    fog_color = mix(fog_color, surface_color, blue_intensity);
    fog_color = mix(fog_color, vec4(vec3(0), 1), 1 - darkness_intensity);
    output_color = mix(fog_color, output_color, fogv * fogv * fogv);
    output_color = vec4(output_color.rgb + clamp(godray.r, 0, 1) * sky_color.rgb, 1.0);
}
