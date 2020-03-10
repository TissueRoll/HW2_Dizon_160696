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

// Struct containing vertex info
struct Vertex
{
	// Position
	float x, y, z;

	// Normal
	float nx, ny, nz;

	// UV coordinates
	float u, v;
};

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

	int windowWidth = 640;
	int windowHeight = 480;

	// Create a GLFW window
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Basic Lighting", nullptr, nullptr);

	// Check window validity
	if (!window)
	{
		std::cerr << "Cannot create window.";
		return -1;
	}

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

	glBindVertexArray(0);

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

	// Create shader program for the cube
	GLuint cubeProgram = CreateShaderProgram("cube.vsh", "cube.fsh");

	// Create shader program for the quad
	GLuint quadProgram = CreateShaderProgram("quad.vsh", "quad.fsh");

	// Construct the projection matrix
	glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), windowWidth * 1.0f / windowHeight, 0.1f, 100.0f);

	// Camera parameters
	glm::vec3 eyePosition = glm::vec3(0.0f, 0.0f, 10.0f);
	float cameraPitch = 0.0f;
	float cameraYaw = -90.0f;
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

	double prevTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		// Calculate amount of time passed since the last frame
		float deltaTime = glfwGetTime() - prevTime;
		prevTime = glfwGetTime();

		// Handle camera look input (up/down)
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			cameraPitch += lookSpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			cameraPitch -= lookSpeed *deltaTime;
		}
		cameraPitch = glm::clamp(cameraPitch, -89.0f, 89.0f);

		// Handle camera look input (left/right)
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			cameraYaw -= lookSpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			cameraYaw += lookSpeed * deltaTime;
		}

		// Calculate the camera's look direction based on the
		// camera's pitch and yaw using spherical coordinates
		glm::vec3 lookDir(0.0f);
		lookDir.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
		lookDir.y = sin(glm::radians(cameraPitch));
		lookDir.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));

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

		// Bind the vao of the cube
		glBindVertexArray(cubeVao);

		// Use the shader for the cube
		glUseProgram(cubeProgram);

		// Pass the projection matrix to the shader
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));

		// Construct the view matrix, and pass the view matrix to the shader
		glm::mat4 viewMatrix = glm::lookAt(eyePosition, eyePosition + lookDir, glm::vec3(0.0f, 1.0f, 0.0f));
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

			// Tell the shader that the diffuse map texture is texture unit 0
			glUniform1i(glGetUniformLocation(cubeProgram, "diffuseTex"), 0);

			// Draw!
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

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
