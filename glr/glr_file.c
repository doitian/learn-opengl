#include <stdlib.h>
#include <stdio.h>

#include "glr.h"

char *glrReadFile(const char *filename, const char *mode, GLsizei *outLen)
{
  char *buffer = NULL;
  long len = -1;

  FILE *file = fopen(filename, mode);
  if (!file)
  {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  len = ftell(file);
  fseek(file, 0, SEEK_SET);

  buffer = (char *)malloc(len + 1);
  if (!buffer)
  {
    return NULL;
  }
  buffer[len] = '\0';

  if (fread(buffer, 1, len, file) != len)
  {
    free(buffer);
    return NULL;
  }

  fclose(file);

  if (outLen != NULL)
  {
    *outLen = (GLsizei)len;
  }
  return buffer;
}
