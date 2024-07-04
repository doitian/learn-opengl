#ifndef __GLR_H__
#define __GLR_H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef struct GlrError
{
  // OpenGL error or `GL_NO_ERROR`
  GLenum glError;
  // GLFW error or `GLFW_NO_ERROR`
  int glfwError;
  // Error message
  const GLubyte *message;
} GlrError;

typedef struct GlrSetupArgs
{
  int windowWidth;
  int windowHeight;
  const char *windowTitle;
} GlrSetupArgs;

/**
 * @brief Setup the OpenGL context and returns the window.
 * @param args The output param to receive the error on failure.
 * @param error The output param to receive the error on failure.
 * @return The window on success or NULL on failure.
 */
GLFWwindow *glrSetup(GlrSetupArgs *args, GlrError *error);

/**
 * @brief Teardown the window and the OpenGL context.
 */
void glrTeardown(GLFWwindow *window);

#endif // __GLR_H__
