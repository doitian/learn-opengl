#ifndef __GLR_H__
#define __GLR_H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef struct GlrError
{
  // OpenGL error or `GL_NO_ERROR`
  GLenum glError;
  // GLFW error or `GLFW_NO_ERROR`
  int glfwError;
  // Error message
  const GLubyte *message;
} GlrError;

typedef struct GlrSetupArgs
{
  int windowWidth;
  int windowHeight;
  const char *windowTitle;
} GlrSetupArgs;

typedef struct GlrShader GlrShader;

/**
 * @brief Setup the OpenGL context and returns the window.
 * @param args The output param to receive the error on failure.
 * @param outError The output param to receive the error on failure.
 * @return The window on success or NULL on failure.
 */
GLFWwindow *glrSetup(GlrSetupArgs *args, GlrError *outError);

/**
 * @brief Teardown the window and the OpenGL context.
 */
void glrTeardown(GLFWwindow *window);

/**
 * @brief Read the file into a buffer.
 *
 * The buffer is allocated in the heap and must be freed by the caller.
 *
 * An extra `\0` is always appended at the end of the buffer on success reading, so the buffer size is `outLen` plus one.
 *
 * @param filename The file name.
 * @param mode Open mode for fopen.
 * @param outLen Output param to get the file size.
 * @return The file content plus an extra `\0` byte or NULL on failure.
 */
const char *glrReadFile(const char *filename, const char *mode, long *outLen);

/**
 * @brief Create a shader by compiling the source code.
 *
 * Parameters `count` and `string` are the same as `glShaderSource`.
 *
 * @return The shader object or 0 on error.
 */
GLuint glrShaderSource(GLenum shaderType, GLsizei count, const GLchar **string, const GLint *length);

/**
 * @brief Create a shader by loading the binary.
 *
 * Parameters are the same as `glShaderBInary`.
 *
 * @return The shader object or 0 on error.
 */
GLuint glrShaderBinary(GLenum shaderType, GLenum binaryFormat, const void *binary, GLsizei length, const GLchar *entryPoint);

/**
 * @brief Get the shader source compilation or binary specialization error.
 *
 * @return The error message or NULL if no error. The caller is responsible for freeing the memory.
 */
const GLchar *glrGetShaderError(GLuint shader);

#endif // __GLR_H__
