#include "glProgramPipeline.hpp"
#include "cstdio"

mental::GLProgramPipeline::GLProgramPipeline(const GLShaderProgram* programs, size_t numPrograms)
{
	glCreateProgramPipelines(1, &handle_);
	for (size_t i = 0; i < numPrograms; i++)
	{
		glUseProgramStages(handle_, programs[i].getTypeBit(), programs[i].getHandle());
	}

	GLint success;
	glValidateProgramPipeline(handle_);
	glGetProgramPipelineiv(handle_, GL_VALIDATE_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetProgramPipelineInfoLog(handle_, 512, nullptr, infoLog);
		printf("Pipeline validation failed:\n%s\n", infoLog);
	}
}

mental::GLProgramPipeline::~GLProgramPipeline()
{
	glDeleteProgramPipelines(1, &handle_);
}

void mental::GLProgramPipeline::use() const
{
	glBindProgramPipeline(handle_);
}
