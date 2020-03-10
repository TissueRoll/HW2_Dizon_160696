#version 330

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

out vec3 fragPos;
out vec3 outNormal;
out vec2 outUV;
out mat3 TBN;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main() {
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);

    fragPos = vec3(modelMatrix * vec4(vertexPosition, 1.0));

    outNormal = mat3(transpose(inverse(modelMatrix))) * vertexNormal;

    outUV = vertexUV;

    vec3 T = normalize(vec3(modelMatrix * vec4(vertexTangent, 0.0)));
    vec3 B = normalize(vec3(modelMatrix * vec4(vertexBitangent, 0.0)));
    vec3 N = normalize(vec3(modelMatrix * vec4(vertexNormal, 0.0)));
    TBN = mat3(T, B, N);
}