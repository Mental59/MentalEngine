#include <cassert>
#include "glShaderProgram.hpp"
#include "utils/utils.hpp"

GLShaderProgram::GLShaderProgram(const char* fileName)
	: GLShaderProgram(glShaderTypeFromFilename(fileName), readShaderFile(fileName).c_str(), fileName)
{
}

GLShaderProgram::GLShaderProgram(GLenum type, const char* text, const char* fileName)
	: type_(type),
	typeBit_(glShaderTypeBitFromType(type_)),
	handle_(glCreateProgram())
{
	GLuint shaderHandle = createShader(text, fileName);

	glProgramParameteri(handle_, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glAttachShader(handle_, shaderHandle);
	glLinkProgram(handle_);
	glDetachShader(handle_, shaderHandle);

	int programLinkStatus;
	glGetProgramiv(handle_, GL_LINK_STATUS, &programLinkStatus);
	if (programLinkStatus == GL_FALSE)
	{
		char buffer[8192];
		GLsizei length = 0;
		glGetProgramInfoLog(handle_, sizeof(buffer), &length, buffer);
		glDeleteProgram(handle_);
		glDeleteShader(shaderHandle);

		if (length)
		{
			printf("Error in shader %s\n: %s\n", fileName, buffer);
		}

		assert(false);
	}

	glDeleteShader(shaderHandle);
}

GLShaderProgram::~GLShaderProgram()
{
	glDeleteProgram(handle_);
}

GLuint GLShaderProgram::createShader(const char* text, const char* fileName)
{
	GLuint shaderHandle = glCreateShader(type_);
	glShaderSource(shaderHandle, 1, &text, nullptr);
	glCompileShader(shaderHandle);
	GLint compileStatus;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		char buffer[8192];
		GLsizei length = 0;
		glGetShaderInfoLog(shaderHandle, sizeof(buffer), &length, buffer);
		glDeleteShader(shaderHandle);

		if (length)
		{
			printf("%s (File: %s)\n", buffer, fileName);
			printShaderSource(text);
		}

		assert(false);
	}
	return shaderHandle;
}

GLenum glShaderTypeFromFilename(const char* fileName)
{
	if (endsWith(fileName, ".vert"))
		return GL_VERTEX_SHADER;

	if (endsWith(fileName, ".frag"))
		return GL_FRAGMENT_SHADER;

	if (endsWith(fileName, ".geom"))
		return GL_GEOMETRY_SHADER;

	if (endsWith(fileName, ".tesc"))
		return GL_TESS_CONTROL_SHADER;

	if (endsWith(fileName, ".tese"))
		return GL_TESS_EVALUATION_SHADER;

	if (endsWith(fileName, ".comp"))
		return GL_COMPUTE_SHADER;

	assert(false);

	return 0;
}

GLbitfield glShaderTypeBitFromType(GLenum type)
{
	if (type == GL_VERTEX_SHADER)
		return GL_VERTEX_SHADER_BIT;

	if (type == GL_FRAGMENT_SHADER)
		return GL_FRAGMENT_SHADER_BIT;

	if (type == GL_GEOMETRY_SHADER)
		return GL_GEOMETRY_SHADER_BIT;

	if (type == GL_TESS_CONTROL_SHADER)
		return GL_TESS_CONTROL_SHADER_BIT;

	if (type == GL_TESS_EVALUATION_SHADER)
		return GL_TESS_EVALUATION_SHADER_BIT;

	if (type == GL_COMPUTE_SHADER)
		return GL_COMPUTE_SHADER_BIT;

	assert(false);

	return 0;
}
