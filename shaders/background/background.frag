#version 450

uniform vec3 cameraPos;

vec4 fog_color = vec4(.0, .25, .8, 1);
vec4 surface_color = vec4(.0, .55, .7, 1);


layout(location=0) out vec4 output_color;

void main() {
    float cameraDepth = cameraPos.y;

    float sun_intensity = clamp(exp(.025 * cameraDepth), 0, 1);
    float fogv2 = clamp(exp(.0045 * cameraDepth), 0, 1);
    fog_color = mix(fog_color, surface_color, sun_intensity);
    output_color = mix(fog_color, vec4(vec3(0), 1), 1 - fogv2);

    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    output_color = vec4(output_color.xyz, z);
}
