#include <stdio.h>
#include <glr.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/util.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define POINT_LIGHTS_COUNT 4

typedef struct Camera
{
  vec3 position;
  vec3 front;
  vec3 up;
  float fov;
} Camera;
typedef struct DirLight
{
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
} DirLight;
typedef struct PointLight
{
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float constant;
  float linear;
  float quadratic;
} PointLight;
typedef struct SpotLight
{
  vec3 position;
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float constant;
  float linear;
  float quadratic;
  float cutOff;
  float outerCutOff;
} SpotLight;
typedef struct State
{
  Camera camera;
  DirLight dirLight;
  PointLight pointLights[POINT_LIGHTS_COUNT];
  SpotLight spotLight;
} State;

typedef struct DirLightLocation
{
  GLint direction;
  GLint ambient;
  GLint diffuse;
  GLint specular;
} DirLightLocation;
typedef struct PointLightLocation
{
  GLint position;
  GLint ambient;
  GLint diffuse;
  GLint specular;
  GLint constant;
  GLint linear;
  GLint quadratic;
} PointLightLocation;
typedef struct SpotLightLocation
{
  GLint position;
  GLint direction;
  GLint ambient;
  GLint diffuse;
  GLint specular;
  GLint constant;
  GLint linear;
  GLint quadratic;
  GLint cutOff;
  GLint outerCutOff;
} SpotLightLocation;

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
  ensureNoErrorMessage("Compiling Vertex Shader", glrShaderSourceFromFile(vertexShader, "shaders/c17-1.vert"));

  GLuint lightProgram = glCreateProgram();
  {
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(fragShader, "shaders/c17-1.light.frag"));
    glAttachShader(lightProgram, vertexShader);
    glAttachShader(lightProgram, fragShader);
    ensureNoErrorMessage("Linking Program", glrLinkProgram(lightProgram));
    glDeleteShader(fragShader);
  }

  GLuint objectProgram = glCreateProgram();
  {
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(fragShader, "shaders/c17-1.object.frag"));
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

  // positions all containers
  vec3 cubePositions[] = {
      {0.0f, 0.0f, 0.0f},
      {2.0f, 5.0f, -15.0f},
      {-1.5f, -2.2f, -2.5f},
      {-3.8f, -2.0f, -12.3f},
      {2.4f, -0.4f, -3.5f},
      {-1.7f, 3.0f, -7.5f},
      {1.3f, -2.0f, -2.5f},
      {1.5f, 2.0f, -2.5f},
      {1.5f, 0.2f, -1.5f},
      {-1.3f, 1.0f, -1.5f}};

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
      .dirLight = {.direction = {-0.2f, -1.0f, -0.3f}, .ambient = {0.05f, 0.05f, 0.05f}, .diffuse = {0.4f, 0.4f, 0.4f}, .specular = {0.5f, 0.5f, 0.5f}},
      .pointLights = {// point light 1
                      {.position = {0.7f, 0.2f, 2.0f}, .ambient = {0.05f, 0.05f, 0.05f}, .diffuse = {0.8f, 0.8f, 0.8f}, .specular = {1.0f, 1.0f, 1.0f}, .constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f},
                      // point light 2
                      {.position = {2.3f, -3.3f, -4.0f}, .ambient = {0.05f, 0.05f, 0.05f}, .diffuse = {0.8f, 0.8f, 0.8f}, .specular = {1.0f, 1.0f, 1.0f}, .constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f},
                      // point light 3
                      {.position = {-4.0f, 2.0f, -12.0f}, .ambient = {0.05f, 0.05f, 0.05f}, .diffuse = {0.8f, 0.8f, 0.8f}, .specular = {1.0f, 1.0f, 1.0f}, .constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f},
                      // point light 4
                      {.position = {0.0f, 0.0f, -3.0f}, .ambient = {0.05f, 0.05f, 0.05f}, .diffuse = {0.8f, 0.8f, 0.8f}, .specular = {1.0f, 1.0f, 1.0f}, .constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f}},
      .spotLight = {.position = {-0.1f, 0.0f, 5.0f}, .direction = {0.0f, 0.0f, -1.0f}, .ambient = {0.0f, 0.0f, 0.0f}, .diffuse = {1.0f, 1.0f, 1.0f}, .specular = {1.0f, 1.0f, 1.0f}, .constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f, .cutOff = cos(glm_rad(12.5f)), .outerCutOff = cos(glm_rad(17.5f))}};

  glfwSetWindowUserPointer(window, &state);

  glEnable(GL_DEPTH_TEST);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetScrollCallback(window, scrollCallback);

  mat4 model, view, projection;
  char uniformNameBuffer[128];

  GLuint modelLocations[2] = {
      glGetUniformLocation(lightProgram, "model"),
      glGetUniformLocation(objectProgram, "model")};
  GLuint viewLocations[2] = {
      glGetUniformLocation(lightProgram, "view"),
      glGetUniformLocation(objectProgram, "view")};
  GLuint projectionLocations[2] = {
      glGetUniformLocation(lightProgram, "projection"),
      glGetUniformLocation(objectProgram, "projection")};

  GLuint lightColorLocation = glGetUniformLocation(lightProgram, "lightColor");

  GLuint transposedInverseModelLocation = glGetUniformLocation(objectProgram, "transposedInverseModel");
  GLuint viewPosLocation = glGetUniformLocation(objectProgram, "viewPos");
  DirLightLocation dirLightLocation = {
      .direction = glGetUniformLocation(objectProgram, "dirLight.direction"),
      .ambient = glGetUniformLocation(objectProgram, "dirLight.ambient"),
      .diffuse = glGetUniformLocation(objectProgram, "dirLight.diffuse"),
      .specular = glGetUniformLocation(objectProgram, "dirLight.specular")};
  PointLightLocation pointLightsLocations[POINT_LIGHTS_COUNT];
  for (unsigned int i = 0; i < POINT_LIGHTS_COUNT; ++i)
  {
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].position", i);
    pointLightsLocations[i].position = glGetUniformLocation(objectProgram, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].ambient", i);
    pointLightsLocations[i].ambient = glGetUniformLocation(objectProgram, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].diffuse", i);
    pointLightsLocations[i].diffuse = glGetUniformLocation(objectProgram, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].specular", i);
    pointLightsLocations[i].specular = glGetUniformLocation(objectProgram, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].constant", i);
    pointLightsLocations[i].constant = glGetUniformLocation(objectProgram, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].linear", i);
    pointLightsLocations[i].linear = glGetUniformLocation(objectProgram, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].quadratic", i);
    pointLightsLocations[i].quadratic = glGetUniformLocation(objectProgram, uniformNameBuffer);
  }
  SpotLightLocation spotLightLocation = {
      .position = glGetUniformLocation(objectProgram, "spotLight.position"),
      .direction = glGetUniformLocation(objectProgram, "spotLight.direction"),
      .ambient = glGetUniformLocation(objectProgram, "spotLight.ambient"),
      .diffuse = glGetUniformLocation(objectProgram, "spotLight.diffuse"),
      .specular = glGetUniformLocation(objectProgram, "spotLight.specular"),
      .constant = glGetUniformLocation(objectProgram, "spotLight.constant"),
      .linear = glGetUniformLocation(objectProgram, "spotLight.linear"),
      .quadratic = glGetUniformLocation(objectProgram, "spotLight.quadratic"),
      .cutOff = glGetUniformLocation(objectProgram, "spotLight.cutOff"),
      .outerCutOff = glGetUniformLocation(objectProgram, "spotLight.outerCutOff")};

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

    // SpotLight follows camera
    glm_vec3_copy(state.camera.position, state.spotLight.position);
    glm_vec3_copy(state.camera.front, state.spotLight.direction);

    vec3 cameraTarget;
    glm_vec3_add(state.camera.position, state.camera.front, cameraTarget);
    glm_lookat(state.camera.position, cameraTarget, state.camera.up, view);

    glm_perspective(glm_rad(state.camera.fov), 800.0f / 600.0f, 0.1f, 100.0f, projection);

    glUseProgram(objectProgram);

    glUniformMatrix4fv(viewLocations[OBJECT_ID], 1, GL_FALSE, (GLfloat *)view);
    glUniformMatrix4fv(projectionLocations[OBJECT_ID], 1, GL_FALSE, (GLfloat *)projection);

    glUniform3fv(dirLightLocation.direction, 1, state.dirLight.direction);
    glUniform3fv(dirLightLocation.ambient, 1, state.dirLight.ambient);
    glUniform3fv(dirLightLocation.diffuse, 1, state.dirLight.diffuse);
    glUniform3fv(dirLightLocation.specular, 1, state.dirLight.specular);
    for (unsigned int i = 0; i < POINT_LIGHTS_COUNT; ++i)
    {
      glUniform3fv(pointLightsLocations[i].position, 1, state.pointLights[i].position);
      glUniform3fv(pointLightsLocations[i].ambient, 1, state.pointLights[i].ambient);
      glUniform3fv(pointLightsLocations[i].diffuse, 1, state.pointLights[i].diffuse);
      glUniform3fv(pointLightsLocations[i].specular, 1, state.pointLights[i].specular);
      glUniform1f(pointLightsLocations[i].constant, state.pointLights[i].constant);
      glUniform1f(pointLightsLocations[i].linear, state.pointLights[i].linear);
      glUniform1f(pointLightsLocations[i].quadratic, state.pointLights[i].quadratic);
    }
    glUniform3fv(spotLightLocation.position, 1, state.spotLight.position);
    glUniform3fv(spotLightLocation.direction, 1, state.spotLight.direction);
    glUniform3fv(spotLightLocation.ambient, 1, state.spotLight.ambient);
    glUniform3fv(spotLightLocation.diffuse, 1, state.spotLight.diffuse);
    glUniform3fv(spotLightLocation.specular, 1, state.spotLight.specular);
    glUniform1f(spotLightLocation.cutOff, state.spotLight.cutOff);
    glUniform1f(spotLightLocation.outerCutOff, state.spotLight.outerCutOff);
    glUniform1f(spotLightLocation.constant, state.spotLight.constant);
    glUniform1f(spotLightLocation.linear, state.spotLight.linear);
    glUniform1f(spotLightLocation.quadratic, state.spotLight.quadratic);

    glUniform3fv(viewPosLocation, 1, (GLfloat *)(state.camera.position));

    glBindVertexArray(VAOs[OBJECT_ID]);
    for (unsigned int i = 0; i < sizeof(cubePositions) / sizeof(vec3); ++i)
    {
      mat4 cubeModel;
      glm_mat4_identity(cubeModel);
      glm_translate(cubeModel, cubePositions[i]);
      float angle = 20.0f * i;
      glm_rotate(cubeModel, glm_rad(angle), (vec3){1.0f, 0.3f, 0.5f});

      glUniformMatrix4fv(modelLocations[OBJECT_ID], 1, GL_FALSE, (GLfloat *)cubeModel);

      mat4 transposedInverseModel;
      glm_mat4_inv(cubeModel, transposedInverseModel);
      glm_mat4_transpose(transposedInverseModel);
      mat3 transposedInverseModelMat3;
      glm_mat4_pick3(transposedInverseModel, transposedInverseModelMat3);
      glUniformMatrix3fv(transposedInverseModelLocation, 1, GL_FALSE, (GLfloat *)transposedInverseModelMat3);

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glUseProgram(lightProgram);
    glUniformMatrix4fv(viewLocations[LIGHT_ID], 1, GL_FALSE, (GLfloat *)view);
    glUniformMatrix4fv(projectionLocations[LIGHT_ID], 1, GL_FALSE, (GLfloat *)projection);

    glBindVertexArray(VAOs[LIGHT_ID]);
    for (unsigned int i = 0; i < POINT_LIGHTS_COUNT; ++i)
    {
      mat4 lightModel;
      glm_translate_make(lightModel, state.pointLights[i].position);
      glm_scale_uni(lightModel, 0.2f);
      glUniformMatrix4fv(modelLocations[LIGHT_ID], 1, GL_FALSE, (GLfloat *)lightModel);

      glUniform3fv(lightColorLocation, 1, state.pointLights[i].diffuse);

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
