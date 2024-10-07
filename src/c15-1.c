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
  vec3 lightPos;
  int flags;
} State;

enum FLAGS
{
  CHANGE_LIGHT_COLOR = 1 << 0,
  INVERSE_SPECULAR_MAP = 1 << 1,
};

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

void processInput(GLFWwindow *window, float deltaTime, State *state)
{
  Camera *camera = &(state->camera);
  const float cameraSpeed = 2.5f * deltaTime;

  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
  {
    state->flags ^= CHANGE_LIGHT_COLOR;
  }
  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
  {
    state->flags ^= INVERSE_SPECULAR_MAP;
  }

  vec3 translation = {0.0f, 0.0f, 0.0f};

  vec3 front, up, right;
  glm_vec3_copy(camera->front, front);
  glm_vec3_normalize(front);
  glm_vec3_copy(camera->up, up);
  glm_vec3_normalize(up);
  glm_vec3_cross(front, (vec3){0.0f, 1.0f, 0.0f}, right);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  {
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
      glm_vec3_scale(up, cameraSpeed, translation);
    }
    else
    {
      glm_vec3_scale(front, cameraSpeed, translation);
    }
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  {
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
      glm_vec3_scale(up, -cameraSpeed, translation);
    }
    else
    {
      glm_vec3_scale(front, -cameraSpeed, translation);
    }
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
  // hard coded yaw and pitch
  static float lastX = 400.0f, lastY = 300.0f, yaw = -90.0f, pitch = 0.0f;
  static int isFirst = 1;
  if (isFirst)
  {
    isFirst = 0;
    lastX = xpos;
    lastY = ypos;
    // compute the initial yaw and pitch
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

GLenum chooseTextureFormat(int nrChannels)
{
  if (nrChannels == 1)
  {
    return GL_RED;
  }
  if (nrChannels == 4)
  {
    return GL_RGBA;
  }
  return GL_RGB;
}

void loadTexture(GLuint id, GLenum index, const char *path)
{
  int width, height, nrChannels;
  unsigned char *data = ensureStbiSuccess(stbi_load(path, &width, &height, &nrChannels, 0));

  GLenum format = chooseTextureFormat(nrChannels);
  glActiveTexture(index);
  glBindTexture(GL_TEXTURE_2D, id);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(data);
}

int main(int argc, char *argv[])
{
  GlrSetupArgs setup = {.windowWidth = 800, .windowHeight = 600, .windowTitle = argv[0]};
  GlrError error;
  GLFWwindow *window = glrSetup(&setup, &error);

  if (!window)
  {
    fprintf(stderr, "Error: %s\n", error.message);
    return -1;
  }

  const int LIGHT_ID = 0, OBJECT_ID = 1;

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  ensureNoErrorMessage("Compiling Vertex Shader", glrShaderSourceFromFile(vertexShader, "shaders/c15-1.vert"));

  GLuint lightProgram = glCreateProgram();
  {
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(fragShader, "shaders/c15-1.light.frag"));
    glAttachShader(lightProgram, vertexShader);
    glAttachShader(lightProgram, fragShader);
    ensureNoErrorMessage("Linking Program", glrLinkProgram(lightProgram));
    glDeleteShader(fragShader);
  }

  GLuint objectProgram = glCreateProgram();
  {
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(fragShader, "shaders/c15-1.object.frag"));
    glAttachShader(objectProgram, vertexShader);
    glAttachShader(objectProgram, fragShader);
    ensureNoErrorMessage("Linking Program", glrLinkProgram(objectProgram));
    glDeleteShader(fragShader);
  }

  glDeleteShader(vertexShader);

  GLuint textures[2];
  const int DIFFUSE_TEX = 0, SPECULAR_TEX = 1;
  glGenTextures(sizeof(textures) / sizeof(GLuint), textures);
  stbi_set_flip_vertically_on_load(1);
  loadTexture(textures[DIFFUSE_TEX], GL_TEXTURE0, "textures/container2.png");
  loadTexture(textures[SPECULAR_TEX], GL_TEXTURE1, "textures/container2_specular.png");

  glUseProgram(objectProgram);
  glUniform1i(glGetUniformLocation(objectProgram, "material.diffuse"), 0);
  glUniform1i(glGetUniformLocation(objectProgram, "material.specular"), 1);
  glUniform1f(glGetUniformLocation(objectProgram, "material.shininess"), 64.0f);

  float vertices[] = {
      // positions(3f)     // normals(3f)     // texture coords(2f)
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
      0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
      0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
      0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

      -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

      0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
      0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
      0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

  GLuint VBO, VAOs[2];
  glGenBuffers(1, &VBO);
  glGenVertexArrays(2, VAOs);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(VAOs[LIGHT_ID]);
  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(VAOs[OBJECT_ID]);
  // positions
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  // normals
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(sizeof(float) * 3));
  glEnableVertexAttribArray(1);
  // texCoords
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(sizeof(float) * 6));
  glEnableVertexAttribArray(2);

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  State state = {
      .camera = {
          .position = {-0.1f, 0.0f, 5.0f},
          .front = {0.0f, 0.0f, -1.0f},
          .up = {0.0f, 1.0f, 0.0f},
          .fov = 45.0f,
      },
      .lightPos = {1.2f, 1.0f, 2.0f},
  };
  glfwSetWindowUserPointer(window, &state);

  glEnable(GL_DEPTH_TEST);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetScrollCallback(window, scrollCallback);

  mat4 model, view, projection;

  GLuint modelLocations[2] = {
      glGetUniformLocation(lightProgram, "model"),
      glGetUniformLocation(objectProgram, "model")};
  GLuint viewLocations[2] = {
      glGetUniformLocation(lightProgram, "view"),
      glGetUniformLocation(objectProgram, "view")};
  GLuint projectionLocations[2] = {
      glGetUniformLocation(lightProgram, "projection"),
      glGetUniformLocation(objectProgram, "projection")};

  GLuint transposedInverseModelLocation = glGetUniformLocation(objectProgram, "transposedInverseModel");
  GLuint lightPosLocation = glGetUniformLocation(objectProgram, "light.position");
  GLuint viewPosLocation = glGetUniformLocation(objectProgram, "viewPos");
  GLuint flagsLocation = glGetUniformLocation(objectProgram, "flags");

  float lastFrame = glfwGetTime();
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window))
  {
    /* Render here */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput(window, deltaTime, &state);

    vec3 lightColor = {1.0f, 1.0f, 1.0f};
    if ((state.flags & CHANGE_LIGHT_COLOR) != 0)
    {
      lightColor[0] = sin(lastFrame * 2.0f);
      lightColor[1] = sin(lastFrame * 0.7f);
      lightColor[2] = sin(lastFrame * 1.3f);
    }
    vec3 lightSpecular = {1.0f, 1.0f, 1.0f};
    vec3 lightAmbient, lightDiffuse;
    glm_vec3_scale(lightColor, 0.95f, lightDiffuse);
    glm_vec3_scale(lightDiffuse, 0.5f, lightAmbient);

    // Rotate the light
    glm_vec3_rotate(state.lightPos, deltaTime * 0.8f, (vec3){0.0f, 1.0f, 0.0f});

    vec3 cameraTarget;
    glm_vec3_add(state.camera.position, state.camera.front, cameraTarget);
    glm_lookat(state.camera.position, cameraTarget, state.camera.up, view);

    glm_perspective(glm_rad(state.camera.fov), 800.0f / 600.0f, 0.1f, 100.0f, projection);

    glUseProgram(objectProgram);
    glBindVertexArray(VAOs[OBJECT_ID]);
    mat4 cubeModel;
    glm_mat4_identity(cubeModel);
    glUniformMatrix4fv(modelLocations[OBJECT_ID], 1, GL_FALSE, (GLfloat *)cubeModel);
    glUniformMatrix4fv(viewLocations[OBJECT_ID], 1, GL_FALSE, (GLfloat *)view);
    glUniformMatrix4fv(projectionLocations[OBJECT_ID], 1, GL_FALSE, (GLfloat *)projection);

    glUniform3fv(glGetUniformLocation(objectProgram, "light.ambient"), 1, lightAmbient);
    glUniform3fv(glGetUniformLocation(objectProgram, "light.diffuse"), 1, lightDiffuse);
    glUniform3fv(glGetUniformLocation(objectProgram, "light.specular"), 1, lightSpecular);
    glUniform3fv(lightPosLocation, 1, (GLfloat *)(state.lightPos));
    glUniform3fv(viewPosLocation, 1, (GLfloat *)(state.camera.position));
    glUniform1i(flagsLocation, state.flags);

    mat4 transposedInverseModel;
    glm_mat4_inv(cubeModel, transposedInverseModel);
    glm_mat4_transpose(transposedInverseModel);
    mat3 transposedInverseModelMat3;
    glm_mat4_pick3(transposedInverseModel, transposedInverseModelMat3);
    glUniformMatrix3fv(transposedInverseModelLocation, 1, GL_FALSE, (GLfloat *)transposedInverseModelMat3);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glUseProgram(lightProgram);
    glBindVertexArray(VAOs[LIGHT_ID]);
    mat4 lightModel;
    glm_translate_make(lightModel, state.lightPos);
    glm_scale_uni(lightModel, 0.2f);

    glUniformMatrix4fv(modelLocations[LIGHT_ID], 1, GL_FALSE, (GLfloat *)lightModel);
    glUniformMatrix4fv(viewLocations[LIGHT_ID], 1, GL_FALSE, (GLfloat *)view);
    glUniformMatrix4fv(projectionLocations[LIGHT_ID], 1, GL_FALSE, (GLfloat *)projection);

    glUniform3fv(glGetUniformLocation(lightProgram, "lightColor"), 1, lightColor);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glrTeardown(window);
  return 0;
}
