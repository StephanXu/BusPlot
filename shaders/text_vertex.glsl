#version 410
layout(location = 0) in dvec2 vertex;
layout(location = 1) in dvec2 tex;
out vec2 TexCoords;

uniform dmat4 projection;

void main()
{
    gl_Position = vec4(projection * dvec4(vertex, 0.0, 1.0));
    TexCoords = vec2(tex);
}