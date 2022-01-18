#version 450

uniform sampler2D depth_texture;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 cameraPos;

uniform mat4 view_matrix_light;
uniform mat4 projection_matrix_light;
uniform vec3 lightPos;

in vec3 position;
in vec3 normal;

out vec3 newPos;
out vec4 clipPos;
out vec3 oldPos;
// Valeur qui permet de
out vec3 color;

vec2 toTex(vec4 vec) {
    return vec.xy / vec.w * 0.5 + 0.5;
}

const vec3 zero = vec3(0);

void main() {

    clipPos = projection_matrix * view_matrix * vec4(vec3(position), 1.0f);

    mat4 mat = projection_matrix_light * view_matrix_light;

    vec3 worldPos = position;
    oldPos = worldPos;

    // On choppe la coordonnée de l'environment map avec les coordonnées clip space depuis la lumière.
    vec2 coord = toTex(vec4(mat * vec4(position, 1.0)));
    // On discard les vertex invalides directement.
    if (coord.x < 0 || coord.y < 0 || coord.x > 1 || coord.y > 1) {
        color = 0.f.xxx;
        return;
    }

    vec4 env = texture(depth_texture, coord.xy);
    newPos = env.xyz;

    vec3 refracted = normalize(refract(vec3(0, -1.0, 0), normal, 3/4f));
    vec4 viewPos = mat * vec4(worldPos, 1.0f);

    // Raymarching
    int i = 0;
    for (; i < 64; ++i) {
        // Si on depasse la profondeur de l'env, on s'arrête.
        if (env.xyz != zero && env.w < viewPos.z)
            break;
        worldPos += refracted * 6;
        viewPos = mat * vec4(worldPos, 1.0f);
        coord = toTex(viewPos);
        if (coord.x < 0 || coord.y < 0 || coord.x > 1 || coord.y > 1) {
            break;
        }
        env = texture(depth_texture, coord);
    }
    // valeur qui permet de discard les vertex invalides (hors de env ou si le rayon n'a pas été assez loin)
    color = (env.xyz != zero && env.w < viewPos.z ? 1.f : 0.f).xxx;

    newPos = env.xyz;
    gl_Position = projection_matrix * view_matrix * vec4(vec3(newPos), 1.0f);
}
