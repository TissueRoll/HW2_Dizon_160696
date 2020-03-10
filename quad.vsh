#version 330

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;

out vec2 TexCoords;

void main() {
    gl_Position = vec4(vertexPosition, 1.0);
    TexCoords = vertexUV;
}