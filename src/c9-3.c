#include <stdio.h>
#include <glr.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>
#include <cglm/cam.h>

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
  GLFWwindow *window = glrSetup(&setup);

  if (!window)
  {
    fprintf(stderr, "Error: %s\n", glrSetupError());
    return -1;
  }

  GLuint program = glCreateProgram();
  {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    ensureNoErrorMessage("Compiling Vertex Shader", glrShaderSourceFromFile(vertexShader, "shaders/c9-3.vert"));
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(fragShader, "shaders/c9-3.frag"));
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

  mat4 model, view, projection;
  glm_rotate_make(model, glm_rad(50.0f) * (float)glfwGetTime(), (vec3){0.5f, 1.0f, 0.0f});
  glm_translate_make(view, (vec3){0.0f, 0.0f, -3.0f});
  glm_mat4_identity(projection);
  glm_perspective(glm_rad(85.0f), 800.0f / 600.0f, 0.1f, 100.0f, projection);

  GLuint modelLocation = glGetUniformLocation(program, "model");
  glUniformMatrix4fv(modelLocation, 1, GL_FALSE, (GLfloat *)model);
  GLuint viewLocation = glGetUniformLocation(program, "view");
  glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (GLfloat *)view);
  GLuint projectionLocation = glGetUniformLocation(program, "projection");
  glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, (GLfloat *)projection);

  static float vertices[] = {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};
  GLuint VBO, VAO;
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // unbind
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glEnable(GL_DEPTH_TEST);

  vec3 cubePositions[] = {{0.0f, 0.0f, 0.0f}, {2.0f, 5.0f, -15.0f}, {-1.5f, -2.2f, -2.5f}, {-3.8f, -2.0f, -12.3f}, {2.4f, -0.4f, -3.5f}, {-1.7f, 3.0f, -7.5f}, {1.3f, -2.0f, -2.5f}, {1.5f, 2.0f, -2.5f}, {1.5f, 0.2f, -1.5f}, {-1.3f, 1.0f, -1.5f}};

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window))
  {
    /* Render here */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    glm_rotate_make(model, glm_rad(50.0f) * (float)glfwGetTime(), (vec3){0.5f, 1.0f, 0.0f});
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, (GLfloat *)model);

    glBindVertexArray(VAO);
    for (unsigned int i = 0; i < 10; i++)
    {
      mat4 model;
      glm_translate_make(model, cubePositions[i]);
      float angle = 20.0f * i;
      if (i % 3 == 1) {
        angle = (float)glfwGetTime() * 25.0f;
      }
      glm_rotate(model, glm_rad(angle), (vec3){1.0f, 0.3f, 0.5f});
      glUniformMatrix4fv(modelLocation, 1, GL_FALSE, (GLfloat *)model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glrTeardown(window);
  return 0;
}
