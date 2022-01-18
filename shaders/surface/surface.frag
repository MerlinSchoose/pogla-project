#version 450

in vec3 vNormal;
in vec3 ray;

vec4 surface_color = vec4(.0, .55, .7, 1);

layout(location=0) out vec4 output_color;

void main() {
    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    // Un rendu plus realiste avec fresnel et plutot un rayon qui part de l'oeil donnait des resultats decevants.
    // Cache misère pour avoir une impression de relief sur la surface + effet lumineux quand on regarde le ciel.
    vec3 rayn = normalize(ray);
    //
    vec3 refracted = normalize(refract(vec3(0, -1, 0), vNormal, 3/4.f));
    output_color = vec4(surface_color.rgb + pow(dot(refracted, -rayn), 2), z);
}