#include <stdio.h>
#include <glr.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/util.h>

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

typedef struct Uniforms
{
  GLuint model;
  GLuint view;
  GLuint projection;
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

void renderPlane()
{
  static float vertices[] = {
      // positions         // normals        // texcoords
      25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
      -25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,

      25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
      -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
      25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f};
  static GLuint vao = 0, vbo = 0;

  if (vao == 0)
  {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

void renderCube()
{
  static float vertices[] = {
      // back face
      -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
      1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
      1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
      1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
      -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
      -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
      // front face
      -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
      1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
      1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
      1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
      -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
      -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
      // left face
      -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
      -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
      -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
      -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
      -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
      -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
                                                          // right face
      1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
      1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
      1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,    // top-right
      1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
      1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
      1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,    // bottom-left
      // bottom face
      -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
      1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
      1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
      1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
      -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
      -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
      // top face
      -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
      1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
      1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
      1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
      -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
      -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
  };
  static GLuint vao = 0, vbo = 0;

  if (vao == 0)
  {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}

void renderScene(Uniforms *uniforms, GLfloat *view, GLfloat *projection)
{
  glUniformMatrix4fv(uniforms->projection, 1, GL_FALSE, projection);
  glUniformMatrix4fv(uniforms->view, 1, GL_FALSE, view);

  mat4 model = GLM_MAT4_IDENTITY_INIT;

  glm_mat4_identity(model);
  glUniformMatrix4fv(uniforms->model, 1, GL_FALSE, (GLfloat *)model);
  renderPlane();

  glm_translate_make(model, (vec3){0.0f, 1.5f, 0.0f});
  glm_scale_uni(model, 0.5f);
  glUniformMatrix4fv(uniforms->model, 1, GL_FALSE, (GLfloat *)model);
  renderCube();

  glm_translate_make(model, (vec3){2.0f, 0.0f, 1.0});
  glm_scale_uni(model, 0.5f);
  glUniformMatrix4fv(uniforms->model, 1, GL_FALSE, (GLfloat *)model);
  renderCube();

  glm_translate_make(model, (vec3){-1.0f, 0.0f, 2.0});
  vec3 rotationAxis = {1.0f, 0.0f, 1.0f};
  glm_normalize(rotationAxis);
  glm_rotate(model, glm_rad(60.0f), rotationAxis);
  glm_scale_uni(model, 0.25);
  glUniformMatrix4fv(uniforms->model, 1, GL_FALSE, (GLfloat *)model);
  renderCube();
}

int main(int argc, char *argv[])
{
  GlrSetupArgs setup = {.windowWidth = 1600, .windowHeight = 1200, .windowTitle = argv[0]};
  GLFWwindow *window = glrSetup(&setup);

  if (!window)
  {
    fprintf(stderr, "Error: %s\n", glrSetupError());
    return -1;
  }

  State state = {
      .camera = {
          .position = {0.0f, 0.0f, 8.0f},
          .front = {0.0f, 0.0f, -1.0f},
          .up = {0.0f, 1.0f, 0.0f},
          .fov = 45.0f,
      },
  };
  glfwSetWindowUserPointer(window, &state);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetScrollCallback(window, scrollCallback);

  // glEnable(GL_DEPTH_TEST);

  GLuint depthProgram = glCreateProgram();
  {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    ensureNoErrorMessage("Compiling Vertex Shader", glrShaderSourceFromFile(vertexShader, "shaders/c35-1.depth.vert"));
    glAttachShader(depthProgram, vertexShader);

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    ensureNoErrorMessage("Compiling Frag Shader", glrShaderSourceFromFile(fragShader, "shaders/c35-1.depth.frag"));
    glAttachShader(depthProgram, fragShader);

    ensureNoErrorMessage("Linking Program", glrLinkProgram(depthProgram));
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
  }
  Uniforms uniforms = {
      .model = glGetUniformLocation(depthProgram, "model"),
      .view = glGetUniformLocation(depthProgram, "view"),
      .projection = glGetUniformLocation(depthProgram, "projection"),
  };

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

    vec3 cameraTarget;
    glm_vec3_add(state.camera.position, state.camera.front, cameraTarget);
    glm_lookat(state.camera.position, cameraTarget, state.camera.up, view);
    glm_perspective(glm_rad(state.camera.fov), (float)(setup.windowWidth) / setup.windowHeight, 0.1f, 100.0f, projection);

    glUseProgram(depthProgram);
    renderScene(&uniforms, (GLfloat *)view, (GLfloat *)projection);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);
    /* Poll for and process events */
    glfwPollEvents();
  }

  glrTeardown(window);
  return 0;
}
