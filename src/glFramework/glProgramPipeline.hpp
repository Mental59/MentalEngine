#pragma once
#include <glad/gl.h>
#include "glFramework/glShaderProgram.hpp"

namespace mental
{
class GLProgramPipeline
{
public:
	GLProgramPipeline(const GLShaderProgram* programs, size_t numPrograms);
	~GLProgramPipeline();

	void use() const;
	GLuint getHandle() const { return handle_; }

private:
	GLuint handle_;
};
}  // namespace mental
