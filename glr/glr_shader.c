#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "glr.h"

static const GLchar *glrGetShaderError(GLuint shader)
{
  // Specialization is equivalent to compilation.
  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

  if (isCompiled == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    GLchar *infoLog = (GLchar *)malloc(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog);

    glDeleteShader(shader);

    return infoLog;
  }

  return NULL;
}

static const GLchar* strerrorDup()
{
  const char *error = strerror(errno);
  size_t len = strlen(error);
  GLchar *copy = (GLchar *)malloc(len + 1);
  memcpy(copy, error, len);
  copy[len] = '\0';
  return copy;
}

const GLchar* glrShaderSource(GLuint shader, const GLchar *string, GLsizei length)
{
  GLint lengthInt = (GLint)length;
  glShaderSource(shader, 1, &string, &lengthInt);
  glCompileShader(shader);

  return glrGetShaderError(shader);
}

const GLchar* glrShaderSourceFromFile(GLuint shader, const char *filename)
{
  GLsizei len = 0;
  const char *buffer = glrReadFile(filename, "r", &len);
  if (buffer == NULL)
  {
    return strerrorDup();
  }
  return glrShaderSource(shader, buffer, len);
}

const GLchar* glrShaderBinary(GLuint shader, GLenum binaryFormat, const void *binary, GLsizei length, const GLchar *entryPoint)
{
  glShaderBinary(1, &shader, binaryFormat, binary, length);
  glSpecializeShader(shader, entryPoint, 0, NULL, NULL);

  return glrGetShaderError(shader);
}

const GLchar* glrShaderBinaryFromFile(GLuint shader, GLenum binaryFormat, const char *filename, const GLchar *entryPoint)
{
  GLsizei len = 0;
  const char *buffer = glrReadFile(filename, "rb", &len);
  if (buffer == NULL)
  {
    return strerrorDup();
  }
  return glrShaderBinary(shader, binaryFormat, buffer, len, entryPoint);
}

const GLchar *glrLinkProgram(GLuint program) {
  glLinkProgram(program);

  GLint isLinked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &isLinked);

  if (isLinked == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

    GLchar *infoLog = (GLchar *)malloc(maxLength);
    glGetProgramInfoLog(program, maxLength, NULL, infoLog);

    glDeleteProgram(program);

    return infoLog;
  }

  return NULL;
}
