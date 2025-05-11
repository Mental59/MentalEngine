#pragma once
#include "glFramework/glShaderProgram.hpp"
#include <glad/gl.h>

namespace glFramework {
class GLProgramPipeline {
public:
  GLProgramPipeline(const GLShaderProgram* programs, size_t numPrograms);
  ~GLProgramPipeline();

  void use() const;
  GLuint getHandle() const { return handle_; }

private:
  GLuint handle_;
};
} // namespace glFramework
