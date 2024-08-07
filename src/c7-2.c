#include <stdio.h>
#include <glr.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static void ensureNoErrorMessage(const GLchar *prompt, const GLchar *message)
{
  if (message)
  {
    fprintf(stderr, "%s: %s\n", prompt, message);
    free((void *)message);
    exit(-1);
  }
}

static unsigned char *ensureStbiSuccess(unsigned char *data)
{
  if (data)
  {
    return data;
  }

  fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
  exit(-1);
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

  GLuint program = glCreateProgram();
  {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    ensureNoErrorMessage("Compiling Vertex Shader", glrShaderSourceFromFile(vertexShader, "shaders/c7-1.vert"));
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(fragShader, "shaders/c7-1.frag"));
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragShader);
    ensureNoErrorMessage("Linking Program", glrLinkProgram(program));
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
  }

  GLuint textures[2];
  glGenTextures(2, textures);
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(1);
  {
    unsigned char *data = ensureStbiSuccess(stbi_load("textures/container.jpg", &width, &height, &nrChannels, 0));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
  }
  {
    unsigned char *data = ensureStbiSuccess(stbi_load("textures/awesomeface.png", &width, &height, &nrChannels, 0));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
  }

  glUseProgram(program);
  glUniform1i(glGetUniformLocation(program, "texture1"), 0);
  glUniform1i(glGetUniformLocation(program, "texture2"), 1);

  static float vertices[] = {
      // pos: x, y, z, tex: u, v
      0.5f, 0.5f, 0.0f, 2.0f, 2.0f,   // top right
      0.5f, -0.5f, 0.0f, 2.0f, 0.0f,  // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
      -0.5f, 0.5f, 0.0f, 0.0f, 2.0f   // top left
  };
  static unsigned int indices[] = {0, 1, 3,
                                   1, 2, 3};
  GLuint VBO, VAO, EBO;
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // unbind
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glrTeardown(window);
  return 0;
}
