#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>

// Reads the contents of the file specified by the file path,
// and places the file contents into a string.
// @param	filePath		Path of the file to read
// @param	outputString	String where the file contents will be copied to
// @return	Returns true if the file was successfully read or not.
bool ReadFile(const std::string& filePath, std::string& outputString) {
	std::ifstream file(filePath);
	if (file.fail()) {
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		outputString += line + "\n";
	}

	return true;
}

// Creates a shader object given the shader type and the corresponding shader source
// @param	type	Shader type (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, ...)
// @param	source	Shader source (as string)
// @return	Returns the handle to the shader object
GLuint CreateShader(const GLuint& type, const std::string& source)
{
	// Create the shader object of the given type
	GLuint shader = glCreateShader(type);

	// Compile the shader source
	const char* sourceCStr = source.c_str();
	GLint sourceLen = source.size();
	glShaderSource(shader, 1, &sourceCStr, &sourceLen);
	glCompileShader(shader);

	// Check compilation status
	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetShaderInfoLog(shader, infoLogLen, &infoLogLen, infoLog);

		std::string errorMsg;

		if (type == GL_VERTEX_SHADER)
		{
			errorMsg += std::string("Failed to compile vertex shader!\n");
		}
		else if (type == GL_FRAGMENT_SHADER)
		{
			errorMsg += std::string("Failed to compile fragment shader!\n");
		}
		else
		{
			errorMsg += std::string("Failed to compile shader!\n");
		}
		errorMsg += std::string(infoLog);

		std::cout << errorMsg << std::endl;
	}

	return shader;
}

GLuint CreateShaderProgramFromSource(const std::string& vertexShaderSource, const std::string& fragmentShaderSource)
{
	// Create the vertex and fragment shader objects
	GLuint vsh = CreateShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fsh = CreateShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	// Create a shader program object
	GLuint program = glCreateProgram();

	// Attach the vertex and fragment shaders to the program
	glAttachShader(program, vsh);
	glAttachShader(program, fsh);

	// Link all attached shaders
	glLinkProgram(program);

	// Check shader program link status
	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetProgramInfoLog(program, infoLogLen, &infoLogLen, infoLog);
		throw std::runtime_error(std::string("program link error: ") + infoLog);
		return 0;
	}

	// Detach the vertex and fragment shaders from the program,
	// since they've already been linked and we don't need them anymore.
	glDetachShader(program, vsh);
	glDetachShader(program, fsh);

	// Delete the vertex and fragment shader objects
	glDeleteShader(vsh);
	glDeleteShader(fsh);

	return program;
}

// Creates a shader program based on the given vertex and fragment shader source file paths
// @param	vertexShaderPath	Path to the vertex shader file
// @param	fragmentShaderPath	Path to the fragment shader file
// @return	Returns the handle to the shader program
GLuint CreateShaderProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	// Read the source code from the vertex shader file
	std::string vshCode;
	if (!ReadFile(vertexShaderPath, vshCode))
	{
		std::cout << "Failed to read shader file " << vertexShaderPath << std::endl;
		throw std::runtime_error(std::string("failed to read shader file: ") + vertexShaderPath);
		return 0;
	}

	// Read the source code from the fragment shader file
	std::string fshCode;
	if (!ReadFile(fragmentShaderPath, fshCode))
	{
		std::cout << "Failed to read shader file: " << fragmentShaderPath << std::endl;
		throw std::runtime_error(std::string("failed to read shader file: ") + fragmentShaderPath);
		return 0;
	}

	return CreateShaderProgramFromSource(vshCode, fshCode);
}

GLint GetUniformLocation(GLuint program, std::string arrayName, int index)
{
	std::string uniformName = arrayName + "[" + std::to_string(index) + "]";
}
