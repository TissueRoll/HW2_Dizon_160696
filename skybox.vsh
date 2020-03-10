#version 330

layout(location = 0) in vec3 vertexPosition;

out vec3 textureDir;

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main() {
    gl_Position = projMatrix * viewMatrix * vec4(vertexPosition, 1.0);
    textureDir = vertexPosition;
}