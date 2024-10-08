#include "glr.h"

const char* UNKNOWN_GLR_SETUP_ERROR = "Unknown GLR Setup Error";
static GLenum glewError = GLEW_OK;

GLFWwindow *glrSetup(GlrSetupArgs *args)
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
    return NULL;
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
    return NULL;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  /* Init after GL context is available */
  glewError = glewInit();
  if (GLEW_OK != glewError)
  {
    glfwTerminate();
    return NULL;
  }

  return window;
}

const char* glrSetupError() {
  const char* err = NULL;
  if (GLFW_NO_ERROR != glfwGetError(&err) && err != NULL) {
    return err;
  }
  if (GLEW_OK != glewError) {
    GLenum err = glewError;
    glewError = GLEW_OK;
    return glewGetErrorString(err);
  }

  return UNKNOWN_GLR_SETUP_ERROR;
}
