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
typedef struct DirLight
{
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
} DirLight;
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
  SpotLight spotLight;
} State;

typedef struct DirLightUniforms
{
  GLint direction;
  GLint ambient;
  GLint diffuse;
  GLint specular;
} DirLightUniforms;
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
typedef struct Uniforms
{
  GLint model;
  GLint view;
  GLint projection;
  GLint transposedInverseModel;
  GLint viewPos;
  GlrModelMaterialUniforms material;
  DirLightUniforms dirLight;
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

void loadTexture(GLuint id, const char *path)
{
  int width, height, nrChannels;
  unsigned char *data = ensureStbiSuccess(stbi_load(path, &width, &height, &nrChannels, 0));

  GLenum format = chooseTextureFormat(nrChannels);
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
      .dirLight = {.direction = {-0.2f, -1.0f, -0.3f}, .ambient = {0.5f, 0.5f, 0.5f}, .diffuse = {0.9f, 0.9f, 0.9f}, .specular = {0.5f, 0.5f, 0.5f}},
      .spotLight = {.position = {-0.1f, 0.0f, 5.0f}, .direction = {0.0f, 0.0f, -1.0f}, .ambient = {0.0f, 0.0f, 0.0f}, .diffuse = {0.3f, 0.3f, 0.3f}, .specular = {0.5f, 0.5f, 0.5f}, .constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f, .cutOff = cos(glm_rad(15.0f)), .outerCutOff = cos(glm_rad(22.5f))}};

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

  GlrModel *backpack = glrLoadModel("objects/backpack/backpack.obj", loadTexture);
  if (backpack == NULL)
  {
    fprintf(stderr, "Failed to load model backpack\n");
    return -1;
  }
  glrBindModel(backpack);

  mat4 view, projection;
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

    mat4 model, transposedInverseModel;
    mat3 transposedInverseModelMat3;
    glm_scale_make(model, (vec3){0.7f, 0.7f, 0.7f});
    glUniformMatrix4fv(uniforms.model, 1, GL_FALSE, (GLfloat *)model);
    glm_mat4_inv(model, transposedInverseModel);
    glm_mat4_transpose(transposedInverseModel);
    glm_mat4_pick3(transposedInverseModel, transposedInverseModelMat3);
    glUniformMatrix3fv(uniforms.transposedInverseModel, 1, GL_FALSE, (GLfloat *)transposedInverseModelMat3);

    glrDrawModel(backpack, &uniforms.material);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glrTeardown(window);
  return 0;
}
