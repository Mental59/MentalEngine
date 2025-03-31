#pragma once
#include <glad/gl.h>

namespace mental
{
	class GLShaderProgram
	{
	public:
		explicit GLShaderProgram(const char* fileName);
		GLShaderProgram(GLenum type, const char* text, const char* fileName);
		~GLShaderProgram();

		GLenum getType() const { return type_; }
		GLbitfield getTypeBit() const { return typeBit_; }
		GLuint getHandle() const { return handle_; }

	private:
		GLuint createShader(const char* text, const char* fileName);

		GLenum type_;
		GLbitfield typeBit_;
		GLuint handle_;
	};

	GLenum glShaderTypeFromFilename(const char* fileName);
	GLbitfield glShaderTypeBitFromType(GLenum type);

}
