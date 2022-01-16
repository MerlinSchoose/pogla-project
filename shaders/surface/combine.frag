#version 450

uniform sampler2DRect color_texture;
uniform sampler2DRect caustic_texture;

layout(location=0) out vec4 output_color;

void main() {

    vec4 color = texelFetch(color_texture, ivec2(gl_FragCoord.xy));
    vec4 caustic = texelFetch(caustic_texture, ivec2(gl_FragCoord.xy));

    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    if (caustic.x != 0 && (caustic.w <= color.w)) {
        vec3 white = vec3(1);
        vec3 c = white * (caustic.r) + color.rgb;
        output_color = vec4(c, 1.0);
    } else {
        output_color = vec4(color.rgb, 1.0);

    }
}
