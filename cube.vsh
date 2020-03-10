#version 330

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

out vec3 outNormal;
out vec2 outUV;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main() {
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);

    outNormal = mat3(transpose(inverse(modelMatrix))) * vertexNormal;

    outUV = vertexUV;
}