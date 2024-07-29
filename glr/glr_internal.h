#ifndef __GLR_INTERNAL_H__
#define __GLR_INTERNAL_H__

#include "glr.h"

void glrGetGlfwError(GlrError *outError)
{
  if (outError != NULL)
  {
    outError->glError = GL_NO_ERROR;
    outError->glfwError = glfwGetError(&outError->message);
  }
}

void glrSetGlError(GlrError *outError, GLenum glError)
{
  if (outError != NULL)
  {
    outError->glfwError = GLFW_NO_ERROR;
    outError->glError = glError;
    outError->message = glError != GL_NO_ERROR ? glewGetErrorString(glError) : NULL;
  }
}

#endif // __GLR_INTERNAL_H__
