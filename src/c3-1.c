#include <stdio.h>
#include <glr.h>

int main(int argc, char* argv[])
{
  GlrSetupArgs setup = {.windowWidth = 640, .windowHeight = 480, .windowTitle = argv[0]};
  GLFWwindow *window = glrSetup(&setup);

  if (!window)
  {
    fprintf(stderr, "Error: %s\n", glrSetupError());
    return -1;
  }

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window))
  {
    /* Render here */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glrTeardown(window);
  return 0;
}
