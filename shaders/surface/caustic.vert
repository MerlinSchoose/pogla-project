#version 450

uniform sampler2D depth_texture;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 cameraPos;

in vec3 position;
in vec3 normal;
in vec2 uv;

out float depth;
out vec3 newPos;
out vec3 oldPos;
out vec2 vUv;
out vec3 color;

vec2 toTex(vec4 vec) {
    return (vec.xy / vec.w * 0.5 + 0.5);
}

void main() {
    //gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);
    vec4 screenPos = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);

    depth = (model_matrix * vec4(position, 1)).y;

    vec3 worldPos = (model_matrix * vec4(position, 1.0)).xyz;
    oldPos = worldPos;

    vec3 refracted = normalize(refract(vec3(0, 1.0, 0), normal, 3/4f));

    vec4 clipRay = (projection_matrix * view_matrix * vec4(refracted, 1.0));
    vec3 viewRefractedRay = clipRay.xyz;
    //viewRefractedRay.z = -viewRefractedRay.z;

    vec3 viewPos = screenPos.xyz;
    vec2 coord = toTex(vec4(viewPos.xyz, screenPos.w));

    vec4 env = texture(depth_texture, coord.xy);
    newPos = env.xyz;

    gl_Position = projection_matrix * view_matrix * vec4(vec3(oldPos.xyz), 1.0f);
    color = (env.w <= viewPos.z ? 1.f : 0.f).xxx;

    float zMax = 1.f;
    float zMin = 0.f;
    float z = 0.5;
    int i = 0;
    for (; i < 4096; ++i) {
        if (env.w <= viewPos.z)
            break;
        viewPos += viewRefractedRay / length(viewRefractedRay.xy) / 1024.f;
        env = texture2D(depth_texture, toTex(vec4(viewPos.xyz, screenPos.w)));
    }

    newPos = env.xyz;
    gl_Position = projection_matrix * view_matrix * vec4(vec3(newPos), 1.0f);
}
