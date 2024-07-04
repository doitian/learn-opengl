#include "glr_internal.h"

GLFWwindow *glrSetup(GlrError *error)
{
  GLFWwindow *window = NULL;

  /* Initialize the library */
  if (!glfwInit())
  {
    glrGetGlfwError(error);
    return window;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    glrGetGlfwError(error);
    return window;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  /* Init after GL context is available */
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    glrSetGlError(error, err);
    glfwTerminate();
    window = NULL;
  }

  return window;
}
