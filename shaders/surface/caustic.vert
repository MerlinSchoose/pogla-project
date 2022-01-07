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

vec2 toTex(vec4 vec) {
    return (vec.xy / vec.w * 0.5 + 0.5);
}

void main() {
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);

    depth = (model_matrix * vec4(position, 1)).y;

    vec3 worldPos = (model_matrix * vec4(position, 1.0)).xyz;
    oldPos = worldPos;

    vec3 refracted = normalize(refract(vec3(0, -1.0, 0), normal, 3/4.f));

    vec3 viewRefractedRay = (projection_matrix * view_matrix * model_matrix * vec4(refracted, 1.0)).xyz;

    vec3 viewPos = (projection_matrix * view_matrix * model_matrix * vec4(position, 1.0)).xyz;
    vec2 coord = toTex(gl_Position);

    vec4 env = texture(depth_texture, coord.xy);

    vUv = uv;

    for (int i = 0; i < 32; ++i) {
        if (env.w > viewPos.z)
            break;
        viewPos += viewRefractedRay / 2048.0;
        env = texture(depth_texture, toTex(vec4(viewPos, gl_Position.w)));
    }


    newPos = env.xyz;
    gl_Position = projection_matrix * view_matrix * vec4(newPos, 1.0f);
}
