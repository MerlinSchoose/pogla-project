#version 450

uniform mat4 model_matrix;
uniform vec3 cameraPos;

in vec3 intensity;
in vec3 pos;
in vec3 ray;

// Même fonction d'intensité en fonction de la depth que pour les caustiques (logique).
float attenuation = 0.1;
float amplification = 1.7;
float caustic_intensity(vec3 pos) {
    return amplification * exp(-attenuation * -pos.y / 65.f);
}

// Fonction de phase d'atomspheric scattering, qui va augmenter d'intensité dépendant de l'angle entre le vecteur du godray et le vecteur de view.
// Permet de donner l'effet de scattering/halo pour les caustiques.
float mie_phase(float angle) {
    // g Facteur de symmétrie, plus il est eleve plus l'angle (ici directement remplace par le cosinus(angle)) doit etre grand et donc plus le rayon est "fin".
    float g = -0.96;
    float g2 = g * g;
    return (3 * (1 - g2)) / (2 * (2 + g2)) * (1 + angle * angle) / pow(1 + g2 - 2 * g * angle, 3.f/2.f);
}

// fonction d'intensité d'un godray qui diminue en fonction de la depth, de la distance te de l'angle d'incidence avec le viewer.
float godray_intensity(vec3 view, vec3 ray, vec3 pos) {
    float mie = mie_phase((-dot(normalize(view), normalize(ray))));
    return caustic_intensity(pos) * mie * exp(-attenuation * length(view) / 25.f);
}

layout(location=0) out vec4 output_color;

void main() {
    vec3 view = pos - cameraPos;
    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    float res = godray_intensity(view, ray, pos);
    output_color = vec4(res.xxx, z);
}