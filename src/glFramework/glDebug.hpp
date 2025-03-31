#pragma once
#include <glad/gl.h>

namespace mental
{
void messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param);
void initDebug();
}
