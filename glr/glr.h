#ifndef __GLR_H__
#define __GLR_H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
 * @return The window on success or NULL on failure.
 */
GLFWwindow *glrSetup(GlrSetupArgs *args);

/**
 * @brief Get the error message from the last setup error.
 */
const char* glrSetupError();

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
char *glrReadFile(const char *filename, const char *mode, long *outLen);

/**
 * @brief Load a shader by compiling the source code.
 *
 * @param string The source code content.
 * @param length The length of the source code.
 * @return The error message or NULL if no error. The caller is responsible for freeing the memory.
 */
const GLchar *glrShaderSource(GLuint shader, const GLchar *string, GLsizei length);

/**
 * @brief Load a shader by compiling the source code from the file.
 * @return The error message or NULL if no error. The caller is responsible for freeing the memory.
 */
const GLchar *glrShaderSourceFromFile(GLuint shader, const char *filename);

/**
 * @brief Load a shader from the pre-compiled binary.
 *
 * Parameters are the same as `glShaderBinary`.
 *
 * @return The error message or NULL if no error. The caller is responsible for freeing the memory.
 */
const GLchar *glrShaderBinary(GLuint shader, GLenum binaryFormat, const void *binary, GLsizei length, const GLchar *entryPoint);

/**
 * @brief Load a shader from the pre-compiled binary file.
 * @return The error message or NULL if no error. The caller is responsible for freeing the memory.
 */
const GLchar *glrShaderBinaryFromFile(GLuint shader, GLenum binaryFormat, const char *filename, const GLchar *entryPoint);

/**
 * @brief Link the program.
 *
 * @return The error message or NULL if no error. The caller is responsible for freeing the memory.
 */
const GLchar *glrLinkProgram(GLuint program);

#endif // __GLR_H__
