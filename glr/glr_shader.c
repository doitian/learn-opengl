#include "glr.h"

GLuint glrShaderSource(GLenum shaderType, GLsizei count, const GLchar **string, const GLint *length)
{
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, count, string, length);
  glCompileShader(shader);

  return shader;
}

GLuint glrShaderBinary(GLenum shaderType, GLenum binaryFormat, const void *binary, GLsizei length, const GLchar *entryPoint) {
  GLuint shader = glCreateShader(shaderType);
  glShaderBinary(1, &shader, binaryFormat, binary, length);
  glSpecializeShader(shader, entryPoint, 0, NULL, NULL);

  return shader;
}

const GLchar *glrGetShaderError(GLuint shader)
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
