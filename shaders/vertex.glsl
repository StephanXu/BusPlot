#version 410
layout(location = 0) in dvec2 vPos;
layout(location = 1) in vec3 vCol;

uniform dmat4 projection;
uniform dmat4 model;
uniform dmat4 scale;

out vec3 color;

void main() {
    gl_Position =  vec4(projection * model * scale * dvec4(vPos, 0.0, 1.0));
    color = vCol;
}