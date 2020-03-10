#version 330

in vec3 textureDir;

out vec4 fragColor;

uniform samplerCube cubemap;

void main() {
	fragColor = texture(cubemap, textureDir);
}