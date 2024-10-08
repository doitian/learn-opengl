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

typedef struct DirLightUniforms
{
  GLint direction;
  GLint ambient;
  GLint diffuse;
  GLint specular;
} DirLightUniforms;
typedef struct PointLightUniforms
{
  GLint position;
  GLint ambient;
  GLint diffuse;
  GLint specular;
  GLint constant;
  GLint linear;
  GLint quadratic;
} PointLightUniforms;
typedef struct SpotLightUniforms
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
} SpotLightUniforms;
typedef struct MaterialUniforms
{
  GLint diffuse;
  GLint specular;
  GLint shininess;
} MaterialUniforms;
typedef struct Uniforms
{
  GLint model;
  GLint view;
  GLint projection;
  GLint transposedInverseModel;
  GLint viewPos;
  MaterialUniforms material;
  DirLightUniforms dirLight;
  PointLightUniforms pointLights[POINT_LIGHTS_COUNT];
  SpotLightUniforms spotLight;
} Uniforms;

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
  GLFWwindow *window = glrSetup(&setup);

  if (!window)
  {
    fprintf(stderr, "Error: %s\n", glrSetupError());
    return -1;
  }

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

  glfwSetWindowUserPointer(window, &state);

  glEnable(GL_DEPTH_TEST);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetScrollCallback(window, scrollCallback);

  GLuint program = glCreateProgram();
  {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    ensureNoErrorMessage("Compiling Vertex Shader", glrShaderSourceFromFile(vertexShader, "shaders/c21-1.vert"));
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(fragShader, "shaders/c21-1.frag"));
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragShader);
    ensureNoErrorMessage("Linking Program", glrLinkProgram(program));
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
  }

  char uniformNameBuffer[128];
  Uniforms uniforms = {
      .model = glGetUniformLocation(program, "model"),
      .view = glGetUniformLocation(program, "view"),
      .projection = glGetUniformLocation(program, "projection"),
      .transposedInverseModel = glGetUniformLocation(program, "transposedInverseModel"),
      .viewPos = glGetUniformLocation(program, "viewPos"),
      .material = {
          .diffuse = glGetUniformLocation(program, "material.diffuse"),
          .specular = glGetUniformLocation(program, "material.specular"),
          .shininess = glGetUniformLocation(program, "material.shininess")},
      .dirLight = {.direction = glGetUniformLocation(program, "dirLight.direction"), .ambient = glGetUniformLocation(program, "dirLight.ambient"), .diffuse = glGetUniformLocation(program, "dirLight.diffuse"), .specular = glGetUniformLocation(program, "dirLight.specular")},
      .spotLight = {.position = glGetUniformLocation(program, "spotLight.position"), .direction = glGetUniformLocation(program, "spotLight.direction"), .ambient = glGetUniformLocation(program, "spotLight.ambient"), .diffuse = glGetUniformLocation(program, "spotLight.diffuse"), .specular = glGetUniformLocation(program, "spotLight.specular"), .constant = glGetUniformLocation(program, "spotLight.constant"), .linear = glGetUniformLocation(program, "spotLight.linear"), .quadratic = glGetUniformLocation(program, "spotLight.quadratic"), .cutOff = glGetUniformLocation(program, "spotLight.cutOff"), .outerCutOff = glGetUniformLocation(program, "spotLight.outerCutOff")}};
  for (unsigned int i = 0; i < POINT_LIGHTS_COUNT; ++i)
  {
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].position", i);
    uniforms.pointLights[i].position = glGetUniformLocation(program, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].ambient", i);
    uniforms.pointLights[i].ambient = glGetUniformLocation(program, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].diffuse", i);
    uniforms.pointLights[i].diffuse = glGetUniformLocation(program, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].specular", i);
    uniforms.pointLights[i].specular = glGetUniformLocation(program, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].constant", i);
    uniforms.pointLights[i].constant = glGetUniformLocation(program, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].linear", i);
    uniforms.pointLights[i].linear = glGetUniformLocation(program, uniformNameBuffer);
    snprintf(uniformNameBuffer, sizeof(uniformNameBuffer), "pointLights[%d].quadratic", i);
    uniforms.pointLights[i].quadratic = glGetUniformLocation(program, uniformNameBuffer);
  }

  GLuint textures[2];
  const int DIFFUSE_TEX = 0, SPECULAR_TEX = 1;
  glGenTextures(sizeof(textures) / sizeof(GLuint), textures);
  stbi_set_flip_vertically_on_load(1);
  loadTexture(textures[DIFFUSE_TEX], GL_TEXTURE0, "textures/container2.png");
  loadTexture(textures[SPECULAR_TEX], GL_TEXTURE1, "textures/container2_specular.png");

  glUseProgram(program);
  glUniform1i(glGetUniformLocation(program, "material.diffuse"), 0);
  glUniform1i(glGetUniformLocation(program, "material.specular"), 1);
  glUniform1f(glGetUniformLocation(program, "material.shininess"), 64.0f);

  GLuint VBO, VAO;
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(VAO);
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

  mat4 model, view, projection;
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
    processInput(window, deltaTime, &state);

    // SpotLight follows camera
    glm_vec3_copy(state.camera.position, state.spotLight.position);
    glm_vec3_copy(state.camera.front, state.spotLight.direction);

    vec3 cameraTarget;
    glm_vec3_add(state.camera.position, state.camera.front, cameraTarget);
    glm_lookat(state.camera.position, cameraTarget, state.camera.up, view);

    glm_perspective(glm_rad(state.camera.fov), 800.0f / 600.0f, 0.1f, 100.0f, projection);

    glUseProgram(program);

    glUniformMatrix4fv(uniforms.view, 1, GL_FALSE, (GLfloat *)view);
    glUniformMatrix4fv(uniforms.projection, 1, GL_FALSE, (GLfloat *)projection);

    glUniform3fv(uniforms.dirLight.direction, 1, state.dirLight.direction);
    glUniform3fv(uniforms.dirLight.ambient, 1, state.dirLight.ambient);
    glUniform3fv(uniforms.dirLight.diffuse, 1, state.dirLight.diffuse);
    glUniform3fv(uniforms.dirLight.specular, 1, state.dirLight.specular);
    for (unsigned int i = 0; i < POINT_LIGHTS_COUNT; ++i)
    {
      glUniform3fv(uniforms.pointLights[i].position, 1, state.pointLights[i].position);
      glUniform3fv(uniforms.pointLights[i].ambient, 1, state.pointLights[i].ambient);
      glUniform3fv(uniforms.pointLights[i].diffuse, 1, state.pointLights[i].diffuse);
      glUniform3fv(uniforms.pointLights[i].specular, 1, state.pointLights[i].specular);
      glUniform1f(uniforms.pointLights[i].constant, state.pointLights[i].constant);
      glUniform1f(uniforms.pointLights[i].linear, state.pointLights[i].linear);
      glUniform1f(uniforms.pointLights[i].quadratic, state.pointLights[i].quadratic);
    }
    glUniform3fv(uniforms.spotLight.position, 1, state.spotLight.position);
    glUniform3fv(uniforms.spotLight.direction, 1, state.spotLight.direction);
    glUniform3fv(uniforms.spotLight.ambient, 1, state.spotLight.ambient);
    glUniform3fv(uniforms.spotLight.diffuse, 1, state.spotLight.diffuse);
    glUniform3fv(uniforms.spotLight.specular, 1, state.spotLight.specular);
    glUniform1f(uniforms.spotLight.cutOff, state.spotLight.cutOff);
    glUniform1f(uniforms.spotLight.outerCutOff, state.spotLight.outerCutOff);
    glUniform1f(uniforms.spotLight.constant, state.spotLight.constant);
    glUniform1f(uniforms.spotLight.linear, state.spotLight.linear);
    glUniform1f(uniforms.spotLight.quadratic, state.spotLight.quadratic);

    glUniform3fv(uniforms.viewPos, 1, (GLfloat *)(state.camera.position));

    glBindVertexArray(VAO);
    mat4 cubeModel;
    glm_mat4_identity(cubeModel);
    glm_rotate(cubeModel, glm_rad(20.0f), (vec3){1.0f, 0.3f, 0.5f});

    glUniformMatrix4fv(uniforms.model, 1, GL_FALSE, (GLfloat *)cubeModel);
    mat4 transposedInverseModel;
    glm_mat4_inv(cubeModel, transposedInverseModel);
    glm_mat4_transpose(transposedInverseModel);
    mat3 transposedInverseModelMat3;
    glm_mat4_pick3(transposedInverseModel, transposedInverseModelMat3);
    glUniformMatrix3fv(uniforms.transposedInverseModel, 1, GL_FALSE, (GLfloat *)transposedInverseModelMat3);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glrTeardown(window);
  return 0;
}
