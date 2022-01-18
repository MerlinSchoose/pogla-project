#version 450

in vec4 vPos;
in float depth;
layout(location=0) out vec4 output_color;

void main() {

    // depth inutile au final car on utilise les world coordinates
    output_color = vec4(vPos.xyz, depth);
}