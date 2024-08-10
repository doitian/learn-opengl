#include <stdio.h>
#include <glr.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/util.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

typedef struct Camera
{
  vec3 position;
  vec3 front;
  vec3 up;
  float fov;
} Camera;
typedef struct State
{
  Camera camera;
} State;

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

void my_glm_lookat(vec3 eye, vec3 center, vec3 up, mat4 dest)
{
  vec3 cameraFront, cameraUp, cameraRight;

  glm_vec3_sub(center, eye, cameraFront);
  glm_vec3_normalize(cameraFront);

  glm_vec3_crossn(cameraFront, up, cameraRight);
  glm_vec3_cross(cameraRight, cameraFront, cameraUp);

  dest[0][0] = cameraRight[0];
  dest[0][1] = cameraUp[0];
  dest[0][2] = -cameraFront[0];
  dest[0][3] = 0.0f;

  dest[1][0] = cameraRight[1];
  dest[1][1] = cameraUp[1];
  dest[1][2] = -cameraFront[1];
  dest[1][3] = 0.0f;

  dest[2][0] = cameraRight[2];
  dest[2][1] = cameraUp[2];
  dest[2][2] = -cameraFront[2];
  dest[2][3] = 0.0f;

  dest[3][0] = -glm_vec3_dot(cameraRight, eye);
  dest[3][1] = -glm_vec3_dot(cameraUp, eye);
  dest[3][2] = glm_vec3_dot(cameraFront, eye);
  dest[3][3] = 1.0f;
}

void processInput(GLFWwindow *window, float deltaTime, Camera *camera)
{
  const float cameraSpeed = 2.5f * deltaTime;

  vec3 translation = {0.0f, 0.0f, 0.0f};

  // Restrict to xz plane
  vec3 front = {camera->front[0], 0.0f, camera->front[2]};
  glm_vec3_normalize(front);
  vec3 right;
  glm_vec3_cross(front, (vec3){0.0f, 1.0f, 0.0f}, right);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  {
    glm_vec3_scale(front, cameraSpeed, translation);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  {
    glm_vec3_scale(front, -cameraSpeed, translation);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  {
    glm_vec3_scale(right, -cameraSpeed, translation);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  {
    glm_vec3_scale(right, cameraSpeed, translation);
  }
  glm_vec3_add(camera->position, translation, camera->position);
}

void cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
  State *state = (State *)glfwGetWindowUserPointer(window);

  const float sensitivity = 0.1f;
  static float lastX = 400.0f, lastY = 300.0f, yaw = -90.0f, pitch = 0.0f;
  static int isFirst = 1;
  if (isFirst) {
    isFirst = 0;
    lastX = xpos;
    lastY = ypos;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed: y ranges bottom to top
  lastX = xpos;
  lastY = ypos;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch = glm_clamp(pitch + yoffset, -89.0f, 89.0f);

  vec3 direction;
  direction[0] = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
  direction[1] = sin(glm_rad(pitch));
  direction[2] = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
  glm_normalize_to(direction, state->camera.front);
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
  State *state = (State *)glfwGetWindowUserPointer(window);
  state->camera.fov = glm_clamp(state->camera.fov + (float)yoffset, 1.0f, 45.0f);
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
    ensureNoErrorMessage("Compiling Vertex Shader", glrShaderSourceFromFile(vertexShader, "shaders/c10-1.vert"));
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(fragShader, "shaders/c10-1.frag"));
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

  static vec3 cubePositions[] = {{0.0f, 0.0f, 0.0f}, {2.0f, 5.0f, -15.0f}, {-1.5f, -2.2f, -2.5f}, {-3.8f, -2.0f, -12.3f}, {2.4f, -0.4f, -3.5f}, {-1.7f, 3.0f, -7.5f}, {1.3f, -2.0f, -2.5f}, {1.5f, 2.0f, -2.5f}, {1.5f, 0.2f, -1.5f}, {-1.3f, 1.0f, -1.5f}};

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

  State state = {
      .camera = {
          .position = {0.0f, 0.0f, 3.0f},
          .front = {0.0f, 0.0f, -1.0f},
          .up = {0.0f, 1.0f, 0.0f},
          .fov = 45.0f,
      }};
  glfwSetWindowUserPointer(window, &state);

  glEnable(GL_DEPTH_TEST);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetScrollCallback(window, scrollCallback);

  mat4 model, view, projection;

  GLuint modelLocation = glGetUniformLocation(program, "model");
  GLuint viewLocation = glGetUniformLocation(program, "view");
  GLuint projectionLocation = glGetUniformLocation(program, "projection");

  float lastFrame = glfwGetTime();
  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window))
  {
    /* Render here */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput(window, deltaTime, &state.camera);

    glUseProgram(program);
    glBindVertexArray(VAO);

    vec3 cameraTarget;
    glm_vec3_add(state.camera.position, state.camera.front, cameraTarget);
    my_glm_lookat(state.camera.position, cameraTarget, state.camera.up, view);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (GLfloat *)view);

    glm_perspective(glm_rad(state.camera.fov), 800.0f / 600.0f, 0.1f, 100.0f, projection);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, (GLfloat *)projection);

    for (unsigned int i = 0; i < 10; i++)
    {
      mat4 model;
      glm_translate_make(model, cubePositions[i]);
      float angle = 20.0f * i;
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
