#include <stdio.h>
#include <glr.h>

static void ensureNoErrorMessage(const GLchar *prompt, const GLchar *message)
{
  if (message)
  {
    fprintf(stderr, "%s: %s\n", prompt, message);
    exit(-1);
  }
}

int main(int argc, char *argv[])
{
  GlrSetupArgs setup = {.windowWidth = 640, .windowHeight = 480, .windowTitle = argv[0]};
  GlrError error;
  GLFWwindow *window = glrSetup(&setup, &error);

  if (!window)
  {
    fprintf(stderr, "Error: %s\n", error.message);
    return -1;
  }

  GLuint programs[2];
  programs[0] = glCreateProgram();
  programs[1] = glCreateProgram();
  {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint yellowFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    ensureNoErrorMessage("Compiling Vertex Shader", glrShaderSourceFromFile(vertexShader, "shaders/c5-1.vert"));
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(fragShader, "shaders/c5-1.frag"));
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(yellowFragShader, "shaders/c5-3.frag"));

    glAttachShader(programs[0], vertexShader);
    glAttachShader(programs[0], fragShader);
    ensureNoErrorMessage("Linking Program", glrLinkProgram(programs[0]));

    glAttachShader(programs[1], vertexShader);
    glAttachShader(programs[1], yellowFragShader);
    ensureNoErrorMessage("Linking Program", glrLinkProgram(programs[1]));

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
    glDeleteShader(yellowFragShader);
  }

  static float vertices[2][9] = {{
                                     -0.5f, 0.0f, 0.0f, // left
                                     0.0f, -0.5f, 0.0f, // right
                                     0.0f, 0.5f, 0.0f   // top
                                 },
                                 {
                                     0.0f, -0.5f, 0.0f, // left
                                     0.5f, 0.5f, 0.0f,  // right
                                     0.0f, 0.5f, 0.0f   // top
                                 }};
  GLuint VBOs[2], VAOs[2];
  glGenBuffers(2, VBOs);
  glGenVertexArrays(2, VAOs);

  for (int i = 0; i < 2; i++)
  {
    glBindVertexArray(VAOs[i]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[i]), vertices[i], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
  }

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window))
  {
    /* Render here */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < 2; i++)
    {
      glUseProgram(programs[i]);
      glBindVertexArray(VAOs[i]);
      glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glrTeardown(window);
  return 0;
}
