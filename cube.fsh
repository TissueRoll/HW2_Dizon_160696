#version 330

in vec3 fragPos;
in vec3 outNormal;
in vec2 outUV;

out vec4 fragColor;

// Diffuse map
uniform sampler2D diffuseTex;

void main() {
	vec3 diffuseColor = texture(diffuseTex, outUV).rgb;
	fragColor = vec4(diffuseColor, 1.0);
}