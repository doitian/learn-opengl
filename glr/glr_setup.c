#include "glr_internal.h"

GLFWwindow *glrSetup(GlrSetupArgs *args, GlrError *outError)
{
  static GlrSetupArgs DEFAULT_ARGS = {
      .windowWidth = 640,
      .windowHeight = 480,
      .windowTitle = "GLR"};
  GLFWwindow *window = NULL;

  if (args == NULL)
  {
    args = &DEFAULT_ARGS;
  }

  /* Initialize the library */
  if (!glfwInit())
  {
    glrGetGlfwError(outError);
    return window;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(
      args->windowWidth,
      args->windowHeight,
      args->windowTitle,
      NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    glrGetGlfwError(outError);
    return window;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  /* Init after GL context is available */
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    glrSetGlError(outError, err);
    glfwTerminate();
    window = NULL;
  }

  return window;
}
