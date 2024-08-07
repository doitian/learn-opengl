#include <stdio.h>
#include <glr.h>

static void ensureNoErrorMessage(const GLchar* prompt, const GLchar *message)
{
  if (message)
  {
    fprintf(stderr, "%s: %s\n", prompt, message);
    free(message);
    exit(-1);
  }
}

int main(int argc, char* argv[])
{
  GlrSetupArgs setup = {.windowWidth = 640, .windowHeight = 480, .windowTitle = argv[0]};
  GlrError error;
  GLFWwindow *window = glrSetup(&setup, &error);

  if (!window)
  {
    fprintf(stderr, "Error: %s\n", error.message);
    return -1;
  }

  GLuint program = glCreateProgram();
  {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    ensureNoErrorMessage("Compiling Vertex Shader", glrShaderSourceFromFile(vertexShader, "shaders/c5-1.vert"));
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(fragShader, "shaders/c5-1.frag"));
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragShader);
    ensureNoErrorMessage("Linking Program", glrLinkProgram(program));
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
  }

  static float vertices[] = {
      -0.5f, -0.5f, 0.0f, // left
      0.0f, -0.5f, 0.0f,  // right
      0.0f, 0.5f, 0.0f,    // top
      0.0f, -0.5f, 0.0f,  // left
      0.5f, 0.5f, 0.0f, // right
      0.0f, 0.5f, 0.0f    // top
  };
  GLuint VBO, VAO;
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0); 

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window))
  {
    /* Render here */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glrTeardown(window);
  return 0;
}
