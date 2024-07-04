#ifndef __GLR_INTERNAL_H__
#define __GLR_INTERNAL_H__

#include "glr.h"

void glrGetGlfwError(GlrError *error)
{
  if (error != NULL)
  {
    error->glError = GL_NO_ERROR;
    error->glfwError = glfwGetError(&error->message);
  }
}

void glrSetGlError(GlrError *error, GLenum glError)
{
  if (error != NULL)
  {
    error->glfwError = GLFW_NO_ERROR;
    error->glError = glError;
    error->message = glError != GL_NO_ERROR ? glewGetErrorString(glError) : NULL;
  }
}

#endif // __GLR_INTERNAL_H__
