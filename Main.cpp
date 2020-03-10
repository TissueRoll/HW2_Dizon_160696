#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

#include "GLUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

const unsigned int windowWidth = 640;
const unsigned int windowHeight = 480;
glm::vec3 lookDir = glm::vec3(0.0f);
bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = (float)windowWidth / 2.0;
float lastY = (float)windowHeight / 2.0;
float fov = 45.0f;
bool normalMappingEnable = true;

// Struct containing vertex info
struct Vertex
{
	// Position
	float x, y, z;

	// Normal
	float nx, ny, nz;

	// UV coordinates
	float u, v;

	// Tangent
	float tx, ty, tz;

	// Bitangent
	float btx, bty, btz;
};

void generateTB(Vertex a[], unsigned int idx[], int n) {
	for (int i = 0; i < n; i += 3) {
		Vertex p0 = a[idx[i]];
		Vertex p1 = a[idx[i + 1]];
		Vertex p2 = a[idx[i + 2]];
		glm::vec3 e1 = glm::vec3(p1.x, p1.y, p1.z) - glm::vec3(p0.x, p0.y, p0.z);
		glm::vec3 e2 = glm::vec3(p2.x, p2.y, p2.z) - glm::vec3(p0.x, p0.y, p0.z);
		float dU1 = p1.u - p0.u;
		float dU2 = p2.u - p0.u;
		float dV1 = p1.v - p0.v;
		float dV2 = p2.v - p0.v;
		float c = 1.0f / (dU1 * dV2 - dU2 * dV1);
		glm::mat2x3 TB = glm::mat2x3(e1, e2) * glm::mat2(dV2, -dV1, -dU2, dU1) * c;
		glm::vec3 tangent(TB[0][0], TB[0][1], TB[0][2]);
		tangent = glm::normalize(tangent);
		glm::vec3 bitangent(TB[1][0], TB[1][1], TB[1][2]);
		bitangent = glm::normalize(bitangent);
		for (int j = 0; j < 3; j++) {
			a[idx[i + j]].tx = tangent.x;
			a[idx[i + j]].ty = tangent.y;
			a[idx[i + j]].tz = tangent.z;
			a[idx[i + j]].btx = bitangent.x;
			a[idx[i + j]].bty = bitangent.y;
			a[idx[i + j]].btz = bitangent.z;
		}
	}
}

void renderCubes(GLuint &cubeVao, GLuint &cubeProgram, glm::vec3 &teyePosition, glm::mat4 &tprojMatrix, glm::vec3 &tlookDir, std::vector<glm::vec3> &cubePositions, GLuint &cubeDiffuseTex, GLuint &cubeSpecularTex, GLuint &cubeNormalTex) {
	// Bind the vao of the cube
	glBindVertexArray(cubeVao);

	// Use the shader for the cube
	glUseProgram(cubeProgram);

	// Pass the eye position vector to the current shader that we're using
	glUniform3f(glGetUniformLocation(cubeProgram, "eyePos"), teyePosition.x, teyePosition.y, teyePosition.z);

	// Pass directional light parameters to the shader
	glUniform3fv(glGetUniformLocation(cubeProgram, "dirLight.direction"), 1, glm::value_ptr(glm::vec3(0.0f, -1.0f, 0.0f)));
	// glUniform3fv(glGetUniformLocation(cubeProgram, "dirLight.direction"), 1, glm::value_ptr(lookDir));
	glUniform3fv(glGetUniformLocation(cubeProgram, "dirLight.ambient"), 1, glm::value_ptr(glm::vec3(0.05f, 0.05f, 0.05f)));
	glUniform3fv(glGetUniformLocation(cubeProgram, "dirLight.diffuse"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
	glUniform3fv(glGetUniformLocation(cubeProgram, "dirLight.specular"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));

	// Pass point light parameters to the shader
	glUniform3fv(glGetUniformLocation(cubeProgram, "pointLight.position"), 1, glm::value_ptr(glm::vec3(0.0f, 5.0f, 0.0f)));
	glUniform3fv(glGetUniformLocation(cubeProgram, "pointLight.ambient"), 1, glm::value_ptr(glm::vec3(0.01f, 0.01f, 0.01f)));
	glUniform3fv(glGetUniformLocation(cubeProgram, "pointLight.diffuse"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
	glUniform3fv(glGetUniformLocation(cubeProgram, "pointLight.specular"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
	glUniform1f(glGetUniformLocation(cubeProgram, "pointLight.kConstant"), 1.0f);
	glUniform1f(glGetUniformLocation(cubeProgram, "pointLight.kLinear"), 0.09f);
	glUniform1f(glGetUniformLocation(cubeProgram, "pointLight.kQuadratic"), 0.032f);

	// Pass spot light parameters to the shader
	// We pass the camera position and direction as the spot light position and direction respectively
	// to emulate a flash light
	// glm::vec3 temp(0.0f, 0.0f, 0.0f);
	glUniform3fv(glGetUniformLocation(cubeProgram, "spotLight.position"), 1, glm::value_ptr(teyePosition));
	glUniform3fv(glGetUniformLocation(cubeProgram, "spotLight.direction"), 1, glm::value_ptr(lookDir));
	glUniform3fv(glGetUniformLocation(cubeProgram, "spotLight.ambient"), 1, glm::value_ptr(glm::vec3(0.1f, 0.1f, 0.1f)));
	glUniform3fv(glGetUniformLocation(cubeProgram, "spotLight.diffuse"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
	glUniform3fv(glGetUniformLocation(cubeProgram, "spotLight.specular"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
	// glUniform3fv(glGetUniformLocation(cubeProgram, "spotLight.ambient"), 1, glm::value_ptr(temp));
	// glUniform3fv(glGetUniformLocation(cubeProgram, "spotLight.diffuse"), 1, glm::value_ptr(temp));
	// glUniform3fv(glGetUniformLocation(cubeProgram, "spotLight.specular"), 1, glm::value_ptr(temp));
	glUniform1f(glGetUniformLocation(cubeProgram, "spotLight.kConstant"), 1.0f);
	glUniform1f(glGetUniformLocation(cubeProgram, "spotLight.kLinear"), 0.09f);
	glUniform1f(glGetUniformLocation(cubeProgram, "spotLight.kQuadratic"), 0.032f);
	glUniform1f(glGetUniformLocation(cubeProgram, "spotLight.cutOffAngle"), glm::radians(12.5f));

	// Pass the projection matrix to the shader
	glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "projMatrix"), 1, GL_FALSE, glm::value_ptr(tprojMatrix));

	// Construct the view matrix, and pass the view matrix to the shader
	glm::mat4 viewMatrix = glm::lookAt(teyePosition, teyePosition + tlookDir, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Render the cubes (since we're currently bound to the framebuffer that we created,
	// we are rendering to that framebuffer instead of the screen)
	for (int i = 0; i < cubePositions.size(); ++i)
	{
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, cubePositions[i]);

		float angle = 20.0f * i;
		modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));


		// Set the active texture unit to 0, and
		// bind the diffuse map texture to it
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeDiffuseTex);

		// Set the active texture unit to 1, and
		// bind the specular map texture to it
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, cubeSpecularTex);

		// Set the active texture unit to 2, and
		// bind the specular map texture to it
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, cubeNormalTex);

		// Tell the shader that the diffuse map texture is texture unit 0
		glUniform1i(glGetUniformLocation(cubeProgram, "diffuseTex"), 0);

		glUniform1i(glGetUniformLocation(cubeProgram, "normalTex"), 2);

		glUniform1i(glGetUniformLocation(cubeProgram, "normalMappingEnable"), (normalMappingEnable ? 1 : 0));

		// glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

		// Draw!
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}
}

void renderWall(GLuint &wallVao, GLuint &wallProgram, glm::vec3 &teyePosition, glm::mat4 &tprojMatrix, glm::vec3 &tlookDir, GLuint &wallDiffuseTex, GLuint &wallSpecularTex, GLuint &wallNormalTex) {
	// __wall__
		// Bind the vao of the wall
	glBindVertexArray(wallVao);

	// Use the shader for the wall
	glUseProgram(wallProgram);

	// Pass the eye position vector to the current shader that we're using
	glUniform3f(glGetUniformLocation(wallProgram, "eyePos"), teyePosition.x, teyePosition.y, teyePosition.z);

	// Pass directional light parameters to the shader
	glUniform3fv(glGetUniformLocation(wallProgram, "dirLight.direction"), 1, glm::value_ptr(glm::vec3(0.0f, -1.0f, 0.0f)));
	// glUniform3fv(glGetUniformLocation(wallProgram, "dirLight.direction"), 1, glm::value_ptr(lookDir));
	glUniform3fv(glGetUniformLocation(wallProgram, "dirLight.ambient"), 1, glm::value_ptr(glm::vec3(0.05f, 0.05f, 0.05f)));
	glUniform3fv(glGetUniformLocation(wallProgram, "dirLight.diffuse"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
	glUniform3fv(glGetUniformLocation(wallProgram, "dirLight.specular"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));

	// Pass point light parameters to the shader
	glUniform3fv(glGetUniformLocation(wallProgram, "pointLight.position"), 1, glm::value_ptr(glm::vec3(0.0f, 5.0f, 0.0f)));
	glUniform3fv(glGetUniformLocation(wallProgram, "pointLight.ambient"), 1, glm::value_ptr(glm::vec3(0.01f, 0.01f, 0.01f)));
	glUniform3fv(glGetUniformLocation(wallProgram, "pointLight.diffuse"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
	glUniform3fv(glGetUniformLocation(wallProgram, "pointLight.specular"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
	glUniform1f(glGetUniformLocation(wallProgram, "pointLight.kConstant"), 1.0f);
	glUniform1f(glGetUniformLocation(wallProgram, "pointLight.kLinear"), 0.09f);
	glUniform1f(glGetUniformLocation(wallProgram, "pointLight.kQuadratic"), 0.032f);

	// Pass spot light parameters to the shader
	// We pass the camera position and direction as the spot light position and direction respectively
	// to emulate a flash light
	// glm::vec3 temp(0.0f, 0.0f, 0.0f);
	glUniform3fv(glGetUniformLocation(wallProgram, "spotLight.position"), 1, glm::value_ptr(teyePosition));
	glUniform3fv(glGetUniformLocation(wallProgram, "spotLight.direction"), 1, glm::value_ptr(lookDir));
	glUniform3fv(glGetUniformLocation(wallProgram, "spotLight.ambient"), 1, glm::value_ptr(glm::vec3(0.1f, 0.1f, 0.1f)));
	glUniform3fv(glGetUniformLocation(wallProgram, "spotLight.diffuse"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
	glUniform3fv(glGetUniformLocation(wallProgram, "spotLight.specular"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
	// glUniform3fv(glGetUniformLocation(wallProgram, "spotLight.ambient"), 1, glm::value_ptr(temp));
	// glUniform3fv(glGetUniformLocation(wallProgram, "spotLight.diffuse"), 1, glm::value_ptr(temp));
	// glUniform3fv(glGetUniformLocation(wallProgram, "spotLight.specular"), 1, glm::value_ptr(temp));
	glUniform1f(glGetUniformLocation(wallProgram, "spotLight.kConstant"), 1.0f);
	glUniform1f(glGetUniformLocation(wallProgram, "spotLight.kLinear"), 0.09f);
	glUniform1f(glGetUniformLocation(wallProgram, "spotLight.kQuadratic"), 0.032f);
	glUniform1f(glGetUniformLocation(wallProgram, "spotLight.cutOffAngle"), glm::radians(12.5f));

	// Pass the projection matrix to the shader
	glUniformMatrix4fv(glGetUniformLocation(wallProgram, "projMatrix"), 1, GL_FALSE, glm::value_ptr(tprojMatrix));

	// Construct the view matrix, and pass the view matrix to the shader
	glm::mat4 viewMatrix = glm::lookAt(teyePosition, teyePosition + tlookDir, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(wallProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
	glUniformMatrix4fv(glGetUniformLocation(wallProgram, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
	// Render the walls (since we're currently bound to the framebuffer that we created,
	// we are rendering to that framebuffer instead of the screen)

	// Set the active texture unit to 0, and
		// bind the diffuse map texture to it
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wallDiffuseTex);

	// Set the active texture unit to 1, and
	// bind the specular map texture to it
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, wallSpecularTex);

	// Set the active texture unit to 2, and
	// bind the specular map texture to it
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, wallNormalTex);

	// Tell the shader that the diffuse map texture is texture unit 0
	glUniform1i(glGetUniformLocation(wallProgram, "diffuseTex"), 0);

	glUniform1i(glGetUniformLocation(wallProgram, "normalTex"), 2);

	glUniform1i(glGetUniformLocation(wallProgram, "normalMappingEnable"), (normalMappingEnable ? 1 : 0));

	// glUniformMatrix4fv(glGetUniformLocation(wallProgram, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Draw!
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	/*
	glUseProgram(skyboxProgram);
	glBindVertexArray(skyboxVAO);

	// Pass the projection matrix to the shader
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));

	// Pass the view matrix to the shader
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glDepthMask(GL_FALSE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
	// draw skybox
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glDepthMask(GL_TRUE);
	// */
	// __wall__
}

void makeCubemap(GLuint& cubeVao, GLuint& cubeProgram, std::vector<glm::vec3>& cubePositions, GLuint& cubeDiffuseTex, GLuint& cubeSpecularTex, GLuint& cubeNormalTex, GLuint& wallVao, GLuint& wallProgram, GLuint& wallDiffuseTex, GLuint& wallSpecularTex, GLuint& wallNormalTex, GLuint& framebuffer, GLuint& framebufferTex, GLuint& rbo) {

	/*
	
	// Generate the framebuffer
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Generate the texture that we will attach to the framebuffer (as color attachment)
	GLuint framebufferTex;
	glGenTextures(1, &framebufferTex);
	glBindTexture(GL_TEXTURE_2D, framebufferTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Attach the texture to the framebuffer as a color attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTex, 0);

	// Generate the renderbuffer object that we will attach to the framebuffer (as depth and stencil attachment)
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Attach the renderbuffer object to the framebuffer as a depth and stencil attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// Check if the framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error! Framebuffer not complete!" << std::endl;
	}

	// Bind to the default framebuffer just so we don't accidentally do anything to the offscreen framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	*/

	// Step 4
	int cubemapSize = 800;
	glGenTextures(1, &framebufferTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, framebufferTex);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, cubemapSize, cubemapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, cubemapSize, cubemapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, cubemapSize, cubemapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, cubemapSize, cubemapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, cubemapSize, cubemapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, cubemapSize, cubemapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	// Step 5
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	///* Generate renderbuffer object *///
	// Step 6
	/* Set camera to have 90 degrees fov and aspect ratio of 1.0 */
	glm::mat4 projMatrix = glm::perspective(glm::radians(90.0f), (cubemapSize * 1.0f) / (cubemapSize * 1.0f), 0.1f, 100.0f);
	/* Set camera position to wherever we're going to render the sphere */
	glm::vec3 eyePosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 tlookDir(0.0f);
	float tYaw = 0.0f, tPitch = 0.0f; // yaw to look around, pitch to look up or down
	tlookDir.x = cos(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	tlookDir.y = sin(glm::radians(tYaw));
	tlookDir.z = sin(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	glm::vec3 rightVec = glm::cross(tlookDir, glm::vec3(0.0f, 1.0f, 0.0f));

	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.

	/* Set the camera to face right/+x axis (1, 0, 0) */
	tYaw = 0.0f;
	tPitch = 0.0f;
	tlookDir.x = cos(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	tlookDir.y = sin(glm::radians(tYaw));
	tlookDir.z = sin(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	rightVec = glm::cross(tlookDir, glm::vec3(0.0f, 1.0f, 0.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, framebufferTex, 0);
	renderCubes(cubeVao, cubeProgram, eyePosition, projMatrix, tlookDir, cubePositions, cubeDiffuseTex, cubeSpecularTex, cubeNormalTex);
	renderWall(wallVao, wallProgram, eyePosition, projMatrix, tlookDir, wallDiffuseTex, wallSpecularTex, wallNormalTex);

	/* Set the camera to face left/-x axis (-1, 0, 0) */
	tYaw = 180.0f;
	tPitch = 0.0f;
	tlookDir.x = cos(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	tlookDir.y = sin(glm::radians(tYaw));
	tlookDir.z = sin(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	rightVec = glm::cross(tlookDir, glm::vec3(0.0f, 1.0f, 0.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, framebufferTex, 0);
	renderCubes(cubeVao, cubeProgram, eyePosition, projMatrix, tlookDir, cubePositions, cubeDiffuseTex, cubeSpecularTex, cubeNormalTex);
	renderWall(wallVao, wallProgram, eyePosition, projMatrix, tlookDir, wallDiffuseTex, wallSpecularTex, wallNormalTex);

	/* Set the camera to face top/+y axis (0, 1, 0) */
	tYaw = 0.0f;
	tPitch = 90.0f;
	tlookDir.x = cos(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	tlookDir.y = sin(glm::radians(tYaw));
	tlookDir.z = sin(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	rightVec = glm::cross(tlookDir, glm::vec3(0.0f, 1.0f, 0.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, framebufferTex, 0);
	renderCubes(cubeVao, cubeProgram, eyePosition, projMatrix, tlookDir, cubePositions, cubeDiffuseTex, cubeSpecularTex, cubeNormalTex);
	renderWall(wallVao, wallProgram, eyePosition, projMatrix, tlookDir, wallDiffuseTex, wallSpecularTex, wallNormalTex);

	/* Set the camera to face bottom/-y axis (0, -1, 0) */
	tYaw = 0.0f;
	tPitch = 90.0f;
	tlookDir.x = cos(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	tlookDir.y = sin(glm::radians(tYaw));
	tlookDir.z = sin(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	rightVec = glm::cross(tlookDir, glm::vec3(0.0f, 1.0f, 0.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, framebufferTex, 0);
	renderCubes(cubeVao, cubeProgram, eyePosition, projMatrix, tlookDir, cubePositions, cubeDiffuseTex, cubeSpecularTex, cubeNormalTex);
	renderWall(wallVao, wallProgram, eyePosition, projMatrix, tlookDir, wallDiffuseTex, wallSpecularTex, wallNormalTex);

	/* Set the camera to face bacl/+z axis (0, 0, 1) */
	tYaw = 90.0f;
	tPitch = 0.0f;
	tlookDir.x = cos(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	tlookDir.y = sin(glm::radians(tYaw));
	tlookDir.z = sin(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	rightVec = glm::cross(tlookDir, glm::vec3(0.0f, 1.0f, 0.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, framebufferTex, 0);
	renderCubes(cubeVao, cubeProgram, eyePosition, projMatrix, tlookDir, cubePositions, cubeDiffuseTex, cubeSpecularTex, cubeNormalTex);
	renderWall(wallVao, wallProgram, eyePosition, projMatrix, tlookDir, wallDiffuseTex, wallSpecularTex, wallNormalTex);

	/* Set the camera to face front/-z axis (0, 0, -1) */
	tYaw = -90.0f;
	tPitch = 0.0f;
	tlookDir.x = cos(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	tlookDir.y = sin(glm::radians(tYaw));
	tlookDir.z = sin(glm::radians(tYaw)) * cos(glm::radians(tPitch));
	rightVec = glm::cross(tlookDir, glm::vec3(0.0f, 1.0f, 0.0f));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, framebufferTex, 0);
	renderCubes(cubeVao, cubeProgram, eyePosition, projMatrix, tlookDir, cubePositions, cubeDiffuseTex, cubeSpecularTex, cubeNormalTex);
	renderWall(wallVao, wallProgram, eyePosition, projMatrix, tlookDir, wallDiffuseTex, wallSpecularTex, wallNormalTex);

	/* Attach renderbuffer to the framebuffer */
	// GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	// Check if the framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error! Framebuffer not complete!" << std::endl;
	}

	// Bind to the default framebuffer just so we don't accidentally do anything to the offscreen framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

int main()
{
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE)
	{
		std::cerr << "Cannot initialize GLFW!" << std::endl;
		return -1;
	}

	// Tell GLFW to use opengl without the deprecated functions (core profile, forward compatible)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Tell GLFW to use OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Create a GLFW window
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Basic Lighting", nullptr, nullptr);

	// Check window validity
	if (!window)
	{
		std::cerr << "Cannot create window.";
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Make the current window as the current context for OpenGL
	glfwMakeContextCurrent(window);

	// Load OpenGL extensions via GLAD
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	// Vertices of the cube.
	// Convention for each face: lower-left, lower-right, upper-right, upper-left
	Vertex cubeVertices[] =
	{
		// Front
		{ -1.0f, -1.0f, 1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f },
		{ 1.0f, -1.0f, 1.0f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,	1.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f },

		// Back
		{ 1.0f, -1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	0.0f, 0.0f },
		{ -1.0f, -1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	1.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	1.0f, 1.0f },
		{ 1.0f, 1.0f, -1.0f,	0.0f, 0.0f, -1.0f,	0.0f, 1.0f },

		// Left
		{ -1.0f, -1.0f, -1.0f,	-1.0f, 0.0f, 0.0f,	0.0f, 0.0f },
		{ -1.0f, -1.0f, 1.0f,	-1.0f, 0.0f, 0.0f,	1.0f, 0.0f },
		{ -1.0f, 1.0f, 1.0f,	-1.0f, 0.0f, 0.0f,	1.0f, 1.0f },
		{ -1.0f, 1.0f, -1.0f,	-1.0f, 0.0f, 0.0f,	0.0f, 1.0f },

		// Right
		{ 1.0f, -1.0f, 1.0f,	1.0f, 0.0f, 0.0f,	0.0f, 0.0f },
		{ 1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 0.0f,	1.0f, 0.0f },
		{ 1.0f, 1.0f, -1.0f,	1.0f, 0.0f, 0.0f,	1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f,		1.0f, 0.0f, 0.0f,	0.0f, 1.0f },

		// Top
		{ -1.0f, 1.0f, 1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f },
		{ 1.0f, 1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f },
		{ -1.0f, 1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f },

		// Bottom
		{ -1.0f, -1.0f, -1.0f,	0.0f, -1.0f, 0.0f,	0.0f, 0.0f },
		{ 1.0f, -1.0f, -1.0f,	0.0f, -1.0f, 0.0f,	1.0f, 0.0f },
		{ 1.0f, -1.0f, 1.0f,	0.0f, -1.0f, 0.0f,	1.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f,	0.0f, -1.0f, 0.0f,	0.0f, 1.0f }
	};


	// Vertex indices for the cube
	unsigned int cubeIndices[] =
	{
		// Front
		0, 1, 2, 2, 3, 0,

		// Back
		4, 5, 6, 6, 7, 4,

		// Left
		8, 9, 10, 10, 11, 8,

		// Right
		12, 13, 14, 14, 15, 12,

		// Top
		16, 17, 18, 18, 19, 16,

		// Bottom
		20, 21, 22, 22, 23, 20
	};

	// make the tangent and bitangent vectors
	generateTB(cubeVertices, cubeIndices, 36);

	// __walls__
	unsigned int wallIndices[36];
	for (int i = 0; i < 36; i++)
		wallIndices[i] = cubeIndices[i];
	Vertex wallVertices[24];
	for (int i = 0; i < 24; i++) {
		wallVertices[i] = cubeVertices[i];
		wallVertices[i].nx = -wallVertices[i].nx;
		wallVertices[i].ny = -wallVertices[i].ny;
		wallVertices[i].nz = -wallVertices[i].nz;
	}
	generateTB(wallVertices, wallIndices, 36);
	// __walls__

	// Vertices of the quad
	Vertex quadVertices[] =
	{
		{ -1.0f, 1.0f, 0.0f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f },
		{ -1.0f, -1.0f, 0.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f },
		{ 1.0f, -1.0f, 0.0f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f,		0.0f, 0.0f, 1.0f,	1.0f, 1.0f }
	};

	// Vertex indices for the quad
	unsigned int quadIndices[] =
	{
		0, 1, 2, 2, 3, 0
	};

	// Construct VBO for the cube
	GLuint cubeVbo;
	glGenBuffers(1, &cubeVbo);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	// Construct EBO (Element Buffer Object) for the cube
	GLuint cubeEbo;
	glGenBuffers(1, &cubeEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	// Construct VAO for the cube
	GLuint cubeVao;
	glGenVertexArrays(1, &cubeVao);
	glBindVertexArray(cubeVao);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEbo);

	// Vertex position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	// Vertex normal attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));

	// UV coordinates attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));

	// Tangent coordinates attribute
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tx));

	// Bitangent coordinates attribute
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, btx));

	glBindVertexArray(0);

	// Create texture handle for the cube's diffuse map
	GLuint cubeDiffuseTex;
	glGenTextures(1, &cubeDiffuseTex);

	// Bind our diffuse texture
	glBindTexture(GL_TEXTURE_2D, cubeDiffuseTex);

	// Set up the parameters for our diffuse texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Read the data for our diffuse map
	int diffuseWidth, diffuseHeight, diffuseNumChannels;
	unsigned char* diffuseData = stbi_load("Textures/container-diffuse.png", &diffuseWidth, &diffuseHeight, &diffuseNumChannels, 0);

	// Upload the diffuse map data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, diffuseWidth, diffuseHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, diffuseData);

	// Since we already uploaded the diffuse map data to opengl, we don't need it anymore.
	stbi_image_free(diffuseData);
	diffuseData = nullptr;

	// Create texture handle for the cube's specular map
	GLuint cubeSpecularTex;
	glGenTextures(1, &cubeSpecularTex);

	// Bind our specular texture
	glBindTexture(GL_TEXTURE_2D, cubeSpecularTex);

	// Set up the parameters for our specular texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Read the data for our specular map
	int specularWidth, specularHeight, specularNumChannels;
	unsigned char* specularData = stbi_load("Textures/container-specular.png", &specularWidth, &specularHeight, &specularNumChannels, 0);

	// Upload the specular map data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, specularWidth, specularHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, specularData);

	// Since we already uploaded the specular map data to opengl, we don't need it anymore.
	stbi_image_free(specularData);
	specularData = nullptr;

	GLuint cubeNormalTex;
	glGenTextures(1, &cubeNormalTex);

	// Bind our diffuse texture
	glBindTexture(GL_TEXTURE_2D, cubeNormalTex);

	// Set up the parameters for our diffuse texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int normalWidth, normalHeight, normalNumChannels;
	unsigned char* normalData = stbi_load("Textures/container-normal.png", &normalWidth, &normalHeight, &normalNumChannels, 0);

	// Upload the normal map data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, normalWidth, normalHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, normalData);

	// Since we already uploaded the normal map data to opengl, we don't need it anymore.
	stbi_image_free(normalData);
	normalData = nullptr;

	// Construct VBO for the quad
	GLuint quadVbo;
	glGenBuffers(1, &quadVbo);
	glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	// Construct EBO (Element Buffer Object) for the quad
	GLuint quadEbo;
	glGenBuffers(1, &quadEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	// Construct VAO for the quad
	GLuint quadVao;
	glGenVertexArrays(1, &quadVao);
	glBindVertexArray(quadVao);

	glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEbo);

	// Vertex position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	// UV coordinates attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));

	// __wall__
	// Construct VBO for the wall
	GLuint wallVbo;
	glGenBuffers(1, &wallVbo);
	glBindBuffer(GL_ARRAY_BUFFER, wallVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);

	// Construct EBO (Element Buffer Object) for the wall
	GLuint wallEbo;
	glGenBuffers(1, &wallEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wallIndices), wallIndices, GL_STATIC_DRAW);

	// Construct VAO for the wall
	GLuint wallVao;
	glGenVertexArrays(1, &wallVao);
	glBindVertexArray(wallVao);

	glBindBuffer(GL_ARRAY_BUFFER, wallVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEbo);

	// Vertex position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	// Vertex normal attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));

	// UV coordinates attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));

	// Tangent coordinates attribute
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tx));

	// Bitangent coordinates attribute
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, btx));

	glBindVertexArray(0);

	// Create texture handle for the wall's diffuse map
	GLuint wallDiffuseTex;
	glGenTextures(1, &wallDiffuseTex);

	// Bind our diffuse texture
	glBindTexture(GL_TEXTURE_2D, wallDiffuseTex);

	// Set up the parameters for our diffuse texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Read the data for our diffuse map
	int wallDiffuseWidth, wallDiffuseHeight, wallDiffuseNumChannels;
	unsigned char* wallDiffuseData = stbi_load("Textures/brickwall.jpg", &wallDiffuseWidth, &wallDiffuseHeight, &wallDiffuseNumChannels, 0);

	// Upload the wallDiffuse map data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wallDiffuseWidth, wallDiffuseHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, wallDiffuseData);

	// Since we already uploaded the wallDiffuse map data to opengl, we don't need it anymore.
	stbi_image_free(wallDiffuseData);
	wallDiffuseData = nullptr;

	// Create texture handle for the wall's specular map
	GLuint wallSpecularTex;
	glGenTextures(1, &wallSpecularTex);

	// Bind our specular texture
	glBindTexture(GL_TEXTURE_2D, wallSpecularTex);

	// Set up the parameters for our specular texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Read the data for our specular map
	int wallSpecularWidth, wallSpecularHeight, wallSpecularNumChannels;
	unsigned char* wallSpecularData = stbi_load("Textures/brickwall.jpg", &wallSpecularWidth, &wallSpecularHeight, &wallSpecularNumChannels, 0);

	// Upload the wallSpecular map data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wallSpecularWidth, wallSpecularHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, wallSpecularData);

	// Since we already uploaded the wallSpecular map data to opengl, we don't need it anymore.
	stbi_image_free(wallSpecularData);
	wallSpecularData = nullptr;

	GLuint wallNormalTex;
	glGenTextures(1, &wallNormalTex);

	// Bind our wallDiffuse texture
	glBindTexture(GL_TEXTURE_2D, wallNormalTex);

	// Set up the parameters for our diffuse texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int wallNormalWidth, wallNormalHeight, wallNormalNumChannels;
	unsigned char* wallNormalData = stbi_load("Textures/brickwall-normal.jpg", &wallNormalWidth, &wallNormalHeight, &wallNormalNumChannels, 0);

	// Upload the wallNormal map data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wallNormalWidth, wallNormalHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, wallNormalData);

	// Since we already uploaded the wallNormal map data to opengl, we don't need it anymore.
	stbi_image_free(wallNormalData);
	wallNormalData = nullptr;
	/*
	// Construct VBO for the cube
	GLuint skyboxVbo;
	glGenBuffers(1, &skyboxVbo);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(simpleCubeVertices), simpleCubeVertices, GL_STATIC_DRAW);

	// Construct EBO (Element Buffer Object) for the cube
	GLuint skyboxEbo;
	glGenBuffers(1, &skyboxEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	// Construct VAO for the cube
	GLuint skyboxVAO;
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEbo);

	// Vertex position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0);

	GLuint cubeMapTex;
	glGenTextures(1, &cubeMapTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	int width, height, nChannels;
	unsigned char* data = stbi_load("Textures/brickwall.jpg", &width, &height, &nChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	data = stbi_load("Textures/brickwall.jpg", &width, &height, &nChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	data = stbi_load("Textures/brickwall.jpg", &width, &height, &nChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	data = stbi_load("Textures/brickwall.jpg", &width, &height, &nChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	data = stbi_load("Textures/brickwall.jpg", &width, &height, &nChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	data = stbi_load("Textures/brickwall.jpg", &width, &height, &nChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	// */
	// __skybox__

	
	// Generate the framebuffer
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Generate the texture that we will attach to the framebuffer (as color attachment)
	GLuint framebufferTex;
	glGenTextures(1, &framebufferTex);
	glBindTexture(GL_TEXTURE_2D, framebufferTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Attach the texture to the framebuffer as a color attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTex, 0);

	// Generate the renderbuffer object that we will attach to the framebuffer (as depth and stencil attachment)
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Attach the renderbuffer object to the framebuffer as a depth and stencil attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// Check if the framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error! Framebuffer not complete!" << std::endl;
	}

	// Bind to the default framebuffer just so we don't accidentally do anything to the offscreen framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// */

	// Create shader program for the cube
	GLuint cubeProgram = CreateShaderProgram("cube.vsh", "cube.fsh");

	// Create shader program for the quad
	GLuint quadProgram = CreateShaderProgram("quad.vsh", "quad.fsh");

	// __wall__
	GLuint wallProgram = CreateShaderProgram("cube.vsh", "cube.fsh");
	// __wall__

	// Construct the projection matrix
	glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), windowWidth * 1.0f / windowHeight, 0.1f, 100.0f);

	// Camera parameters
	glm::vec3 eyePosition = glm::vec3(0.0f, 0.0f, 10.0f);
	float movementSpeed = 10.0f; // 10 distance units per second
	float lookSpeed = 45.0f; // 45 degrees per second

	// Cube positions
	std::vector<glm::vec3> cubePositions;
	cubePositions.push_back(glm::vec3(2.0f, 5.0f, -15.0f));
	cubePositions.push_back(glm::vec3(-1.5f, -2.2f, -2.5f));
	cubePositions.push_back(glm::vec3(-3.8f, -2.0f, -12.3f));
	cubePositions.push_back(glm::vec3(2.4f, -0.4f, -3.5f));
	cubePositions.push_back(glm::vec3(-1.7f, 3.0f, -7.5f));
	cubePositions.push_back(glm::vec3(1.3f, -2.0f, -2.5f));
	cubePositions.push_back(glm::vec3(1.5f, 2.0f, -2.5f));
	cubePositions.push_back(glm::vec3(1.5f, 0.2f, -1.5f));
	cubePositions.push_back(glm::vec3(-1.3f, 1.0f, -1.5f));

	// GLuint framebuffer, framebufferTex, rbo;
	// makeCubemap(cubeVao, cubeProgram, cubePositions, cubeDiffuseTex, cubeSpecularTex, cubeNormalTex, wallVao, wallProgram, wallDiffuseTex, wallSpecularTex, wallNormalTex, framebuffer, framebufferTex, rbo);

	double prevTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		// Calculate amount of time passed since the last frame
		float deltaTime = glfwGetTime() - prevTime;
		prevTime = glfwGetTime();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		// Handle camera look input (up/down)
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			pitch += lookSpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			pitch -= lookSpeed *deltaTime;
		}
		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		// Handle camera look input (left/right)
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			yaw -= lookSpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			yaw += lookSpeed * deltaTime;
		}

		// Calculate the camera's look direction based on the
		// camera's pitch and yaw using spherical coordinates
		glm::vec3 lookDir(0.0f);
		lookDir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		lookDir.y = sin(glm::radians(pitch));
		lookDir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		// Calculate the right facing vector relative to the camera
		// by taking the cross product between the camera's look direction and the global up vector
		glm::vec3 rightVec = glm::cross(lookDir, glm::vec3(0.0f, 1.0f, 0.0f));

		// Handle camera movement
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			eyePosition -= rightVec * movementSpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			eyePosition += rightVec * movementSpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			eyePosition += lookDir * movementSpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			eyePosition -= lookDir * movementSpeed * deltaTime;
		}

		// Bind to the framebuffer that we created
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		// Enable depth testing
		glEnable(GL_DEPTH_TEST);

		// Set background color of the framebuffer to black
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		// Clear the color and depth buffers of the frame buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderCubes(cubeVao, cubeProgram, eyePosition, projMatrix, lookDir, cubePositions, cubeDiffuseTex, cubeSpecularTex, cubeNormalTex);

		renderWall(wallVao, wallProgram, eyePosition, projMatrix, lookDir, wallDiffuseTex, wallSpecularTex, wallNormalTex);

		// makeCubemap(cubeVao, cubeProgram, cubePositions, cubeDiffuseTex, cubeSpecularTex, cubeNormalTex, wallVao, wallProgram, wallDiffuseTex, wallSpecularTex, wallNormalTex, framebuffer, framebufferTex, rbo);

		// makeCubemap(cubeVao, cubeProgram, cubePositions, cubeDiffuseTex, cubeSpecularTex, cubeNormalTex, wallVao, wallProgram, wallDiffuseTex, wallSpecularTex, framebuffer, rbo);

		// Bind to the default framebuffer (the screen)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// We're only going to render a quad in normalized device coordinates,
		// so no need to do depth testing
		glDisable(GL_DEPTH_TEST);

		// Set background color to black
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		// Clear the color buffer of the default framebuffer
		glClear(GL_COLOR_BUFFER_BIT);

		// Bind to the quad VAO
		glBindVertexArray(quadVao);

		// Use the shader for the quad
		glUseProgram(quadProgram);

		// Use the texture that we used as an attachment to the other framebuffer that we created
		// When we rendered the cubes earlier, the rendered scene is stored in this texture.
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebufferTex);
		glUniform1i(glGetUniformLocation(quadProgram, "screenTexture"), 0);

		// Draw the quad
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		// Swap the front and back buffers
		glfwSwapBuffers(window);

		// Poll pending events
		glfwPollEvents();
	}

	// Terminate GLFW
	glfwTerminate();

	return 0;
}

// https://learnopengl.com/Getting-started/Camera
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.2;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	lookDir = glm::normalize(direction);
}

// https://www.glfw.org/docs/3.3.2/input_guide.html
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		normalMappingEnable = !normalMappingEnable;
}