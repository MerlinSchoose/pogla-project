#version 450

uniform sampler2D depth_texture;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 cameraPos;

uniform mat4 model_matrix_light;
uniform mat4 view_matrix_light;
uniform mat4 projection_matrix_light;
uniform vec3 lightPos;

in vec3 position;
in vec3 normal;

out vec3 newPos;
out vec3 oldPos;
out vec3 color;

vec2 toTex(vec4 vec) {
    return vec.xy / vec.w * 0.5 + 0.5;
}

void main() {
    //gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);
    mat4 mat = projection_matrix_light * view_matrix_light * model_matrix_light;
    vec4 screenPos = projection_matrix_light * view_matrix_light * model_matrix_light * vec4(position, 1.0);

    ivec2 size = textureSize(depth_texture, 0);

    vec3 worldPos = (model_matrix_light * vec4(position, 1.0)).xyz;
    oldPos = worldPos;

    vec3 refracted = normalize(refract(vec3(0, -1.0, 0), normal, 4/3f));

    vec4 clipRay = (projection_matrix_light * view_matrix_light * model_matrix_light * vec4(lightPos + refracted - vec3(0, 1.1f, 0), 1.0));
    vec3 viewRefractedRay = clipRay.xyz / clipRay.w;
    viewRefractedRay.z = viewRefractedRay.z;

    vec3 viewPos = screenPos.xyz / screenPos.w;
    viewPos.z = viewPos.z;
    vec2 coord = toTex(vec4(screenPos));

    vec4 env = texture(depth_texture, coord.xy);
    newPos = env.xyz;

    gl_Position = projection_matrix_light * view_matrix_light * vec4(vec3(oldPos.xyz), 1.0f);
    color = 0.2f.xxx; // (env.w <= viewPos.z ? 1.f : 0.f).xxx;

    float zMax = 1.f;
    float zMin = 0.f;
    float z = 0.5;
    int i = 0;
    float step = length(viewRefractedRay);
    for (; i < 64; ++i) {
        if (env.y > worldPos.y)
            break;
        worldPos += refracted * 8;
        coord = toTex(mat * vec4(worldPos, 1.0f));
        if (coord.x < 0 || coord.y < 0 || coord.x > 1 || coord.y > 1) {
            break;
        }
        env = texture(depth_texture, coord);
    }
    color = (env.w > viewPos.z ? 1.f : 0.f).xxx;

    newPos = env.xyz;
    gl_Position = projection_matrix * view_matrix * vec4(vec3(newPos), 1.0f);
    //gl_Position = projection_matrix_light * view_matrix_light * vec4(vec3(newPos), 1.0f);
}
